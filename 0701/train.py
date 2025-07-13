
import os
import pandas as pd
from PIL import Image
import torch
from torch import nn
from torch.utils.data import Dataset, DataLoader
from torchvision import transforms, models
from tqdm import tqdm
import torch.nn.functional as F
from sklearn.model_selection import train_test_split
import numpy as np
from datetime import datetime, timedelta
import json

# ============= 프로젝트 경로 설정 =============
PROJECT_ROOT = "C:/planttest0/"
CSV_PATH = os.path.join(PROJECT_ROOT, "0701", "dataset8_3.3_updated.csv")
IMG_DIR = "C:/planttest0/TS/TS_3.3/TS_3.3"

BATCH_SIZE = 16
EPOCHS = 3
DEVICE = torch.device("cuda" if torch.cuda.is_available() else "cpu")

# 상추 성장 상수
LETTUCE_GROWTH_STAGES = {
    "정식기": (0, 7),      # 0-7일: 심은 직후
    "생육기": (7, 21),     # 7-21일: 주요 성장기
    "수확기": (21, 35)     # 21-35일: 수확 가능
}

# ============= 멀티모달 데이터 전처리 =============
def analyze_multimodal_data(csv_path):
    """이미지, 날짜, 메타데이터를 종합 분석"""
    print("🔍 멀티모달 데이터 분석 중...")
    
    df = pd.read_csv(csv_path)
    print(f"📊 총 데이터: {len(df)}개")
    print(f"📋 컬럼: {list(df.columns)}")
    
    # 1. 날짜 정보 처리
    if 'planting_date' in df.columns:
        df['planting_date'] = pd.to_datetime(df['planting_date'])
        df['photo_date'] = pd.to_datetime(df['photo_date']) if 'photo_date' in df.columns else datetime.now()
        df['days_since_planting'] = (df['photo_date'] - df['planting_date']).dt.days
    else:
        print("⚠️ 심은 날짜 정보가 없습니다. estimated_days_elapsed 사용")
        df['days_since_planting'] = df['estimated_days_elapsed']
    
    # 2. 날짜 기반 성장 단계 예측
    def predict_stage_by_date(days):
        if days <= 7:
            return 0, "정식기"
        elif days <= 21:
            return 1, "생육기"
        else:
            return 2, "수확기"
    
    df['predicted_stage_by_date'] = df['days_since_planting'].apply(lambda x: predict_stage_by_date(x)[0])
    df['predicted_stage_name'] = df['days_since_planting'].apply(lambda x: predict_stage_by_date(x)[1])
    
    # 3. 실제 라벨과 날짜 예측 비교
    stage_map = {"정식기": 0, "생육기": 1, "수확기": 2}
    df['actual_stage'] = df['growth_stage'].map(stage_map)
    
    # 날짜 vs 실제 라벨 일치도 분석
    df['stage_consistency'] = (df['predicted_stage_by_date'] == df['actual_stage']).astype(int)
    consistency_rate = df['stage_consistency'].mean()
    print(f"📅 날짜-라벨 일치도: {consistency_rate:.2%}")
    
    # 4. 수확까지 남은 일수 (더 정확한 계산)
    df['days_to_harvest'] = np.maximum(0, 30 - df['days_since_planting'])
    
    print(f"✅ 데이터 전처리 완료!")
    print(f"📊 성장 단계별 분포:")
    print(df['actual_stage'].value_counts().sort_index())
    
    return df

# ============= 멀티모달 데이터셋 =============
class MultimodalLettuceDataset(Dataset):
    """이미지 + 날짜 + 메타데이터를 결합한 데이터셋"""
    
    def __init__(self, dataframe, img_dir, transform=None):
        self.data = dataframe.reset_index(drop=True)
        self.img_dir = img_dir
        self.transform = transform
        
        # 유효한 이미지만 필터링
        valid_indices = []
        for idx in range(len(self.data)):
            fname = self.data.iloc[idx]['fname']
            img_path = os.path.join(img_dir, fname)
            if os.path.exists(img_path):
                valid_indices.append(idx)
        
        self.data = self.data.iloc[valid_indices].reset_index(drop=True)
        print(f"📷 유효한 데이터: {len(self.data)}개")

    def __len__(self):
        return len(self.data)

    def __getitem__(self, idx):
        row = self.data.iloc[idx]
        
        # 1. 이미지 로드
        img_path = os.path.join(self.img_dir, row["fname"])
        try:
            image = Image.open(img_path).convert("RGB")
        except Exception as e:
            print(f"⚠️ 이미지 로드 오류: {row['fname']}")
            print(f"   경로: {img_path}")
            print(f"   파일 존재: {os.path.exists(img_path)}")
            print(f"   에러: {str(e)}")
            image = Image.new('RGB', (224, 224), color=(34, 139, 34))
        
        if self.transform:
            image = self.transform(image)
        
        # 2. 날짜 정보 (정규화)
        days_since_planting = row['days_since_planting'] / 35.0  # 0-1 범위로 정규화
        days_to_harvest = row['days_to_harvest'] / 35.0
        
        # 3. 메타데이터 피처
        metadata_features = torch.tensor([
            days_since_planting,
            days_to_harvest,
            row['predicted_stage_by_date'] / 2.0,  # 0-1 범위로 정규화
            row['stage_consistency'],
        ], dtype=torch.float32)
        
        # 4. 타겟 값들
        actual_stage = torch.tensor(row['actual_stage'], dtype=torch.long)
        days_left = torch.tensor([row['days_to_harvest']], dtype=torch.float32)
        
        return image, metadata_features, actual_stage, days_left

# ============= 멀티모달 모델 =============
class MultimodalLettuceModel(nn.Module):
    """이미지 + 메타데이터를 결합한 멀티모달 모델"""
    
    def __init__(self, num_stages=3, metadata_dim=4):
        super().__init__()
        
        # 1. 이미지 인코더 (Vision)
        self.vision_encoder = models.mobilenet_v3_small(weights='DEFAULT')
        self.vision_features = self.vision_encoder.features
        self.vision_avgpool = self.vision_encoder.avgpool
        
        # 2. 메타데이터 인코더 (날짜, 환경 정보)
        self.metadata_encoder = nn.Sequential(
            nn.Linear(metadata_dim, 32),
            nn.ReLU(),
            nn.Dropout(0.1),
            nn.Linear(32, 64),
            nn.ReLU(),
            nn.Dropout(0.1)
        )
        
        # 3. 멀티모달 융합 레이어
        vision_dim = 576  # MobileNetV3-small 출력 크기
        fusion_dim = vision_dim + 64
        
        self.fusion_layer = nn.Sequential(
            nn.Linear(fusion_dim, 256),
            nn.ReLU(),
            nn.Dropout(0.2),
            nn.Linear(256, 128),
            nn.ReLU(),
            nn.Dropout(0.1)
        )
        
        # 4. 태스크별 헤드
        self.stage_head = nn.Linear(128, num_stages)      # 성장 단계
        self.days_head = nn.Linear(128, 1)                # 남은 일수
        
        # 5. 어텐션 메커니즘 (비전 vs 메타데이터 가중치)
        self.attention = nn.Sequential(
            nn.Linear(fusion_dim, 64),
            nn.ReLU(),
            nn.Linear(64, 2),  # vision_weight, metadata_weight
            nn.Softmax(dim=1)
        )

    def forward(self, image, metadata):
        batch_size = image.size(0)
        
        # 1. 이미지 특징 추출
        vision_features = self.vision_features(image)
        vision_features = self.vision_avgpool(vision_features)
        vision_features = torch.flatten(vision_features, 1)
        
        # 2. 메타데이터 특징 추출
        metadata_features = self.metadata_encoder(metadata)
        
        # 3. 특징 결합
        combined_features = torch.cat([vision_features, metadata_features], dim=1)
        
        # 4. 어텐션 가중치 계산
        attention_weights = self.attention(combined_features)
        
        # 5. 가중치 적용한 특징 융합
        weighted_vision = vision_features * attention_weights[:, 0:1]
        weighted_metadata = metadata_features * attention_weights[:, 1:2]
        
        # 6. 최종 특징
        final_features = self.fusion_layer(
            torch.cat([weighted_vision, weighted_metadata], dim=1)
        )
        
        # 7. 예측
        stage_pred = self.stage_head(final_features)
        days_pred = self.days_head(final_features)
        
        return stage_pred, days_pred, attention_weights

# ============= 학습 함수 =============
def train_multimodal_model():
    """멀티모달 모델 학습"""
    
    print("🚀 멀티모달 상추 예측 모델 학습 시작!")
    
    # 1. 데이터 로드
    df = analyze_multimodal_data(CSV_PATH)
    
    # 2. 학습/검증 분할
    train_df, val_df = train_test_split(
        df, test_size=0.2, random_state=42, 
        stratify=df['actual_stage']
    )
    
    print(f"📚 학습 데이터: {len(train_df)}개")
    print(f"🔍 검증 데이터: {len(val_df)}개")
    
    # 3. 데이터 변환
    train_transform = transforms.Compose([
        transforms.Resize((256, 256)),
        transforms.RandomCrop(224, padding=4),
        transforms.RandomHorizontalFlip(0.5),
        transforms.ColorJitter(brightness=0.3, contrast=0.3, saturation=0.2, hue=0.1),
        transforms.RandomRotation(15),
        transforms.ToTensor(),
        transforms.Normalize(mean=[0.485, 0.456, 0.406], std=[0.229, 0.224, 0.225])
    ])
    
    val_transform = transforms.Compose([
        transforms.Resize((224, 224)),
        transforms.ToTensor(),
        transforms.Normalize(mean=[0.485, 0.456, 0.406], std=[0.229, 0.224, 0.225])
    ])
    
    # 4. 데이터셋 생성
    train_dataset = MultimodalLettuceDataset(train_df, IMG_DIR, train_transform)
    val_dataset = MultimodalLettuceDataset(val_df, IMG_DIR, val_transform)
    
    train_loader = DataLoader(train_dataset, batch_size=BATCH_SIZE, shuffle=True, num_workers=2)
    val_loader = DataLoader(val_dataset, batch_size=BATCH_SIZE, shuffle=False, num_workers=2)
    
    # 5. 모델 초기화
    model = MultimodalLettuceModel().to(DEVICE)
    
    # 6. 손실 함수 및 옵티마이저
    criterion_stage = nn.CrossEntropyLoss()
    criterion_days = nn.MSELoss()
    
    optimizer = torch.optim.AdamW(model.parameters(), lr=1e-3, weight_decay=1e-4)
    scheduler = torch.optim.lr_scheduler.CosineAnnealingLR(optimizer, T_max=EPOCHS)
    
    # 7. 학습 루프
    best_val_loss = float('inf')
    
    for epoch in range(EPOCHS):
        # 학습
        model.train()
        train_loss = 0
        train_acc = 0
        
        pbar = tqdm(train_loader, desc=f"🌱 Epoch {epoch+1}/{EPOCHS}")
        for images, metadata, stages_true, days_true in pbar:
            images = images.to(DEVICE)
            metadata = metadata.to(DEVICE)
            stages_true = stages_true.to(DEVICE)
            days_true = days_true.to(DEVICE)
            
            # 순전파
            stages_pred, days_pred, attention_weights = model(images, metadata)
            
            # 손실 계산
            loss_stage = criterion_stage(stages_pred, stages_true)
            loss_days = criterion_days(days_pred, days_true)
            
            # 가중 손실 (성장 단계가 더 중요)
            total_loss = 0.7 * loss_stage + 0.3 * loss_days
            
            # 역전파
            optimizer.zero_grad()
            total_loss.backward()
            optimizer.step()
            
            # 통계
            train_loss += total_loss.item()
            _, predicted = torch.max(stages_pred.data, 1)
            train_acc += (predicted == stages_true).sum().item() / len(stages_true)
            
            pbar.set_postfix({
                'Loss': f"{train_loss/len(pbar):.3f}",
                'Acc': f"{train_acc/len(pbar):.3f}"
            })
        
        # 검증
        model.eval()
        val_loss = 0
        val_acc = 0
        
        with torch.no_grad():
            for images, metadata, stages_true, days_true in val_loader:
                images = images.to(DEVICE)
                metadata = metadata.to(DEVICE)
                stages_true = stages_true.to(DEVICE)
                days_true = days_true.to(DEVICE)
                
                stages_pred, days_pred, attention_weights = model(images, metadata)
                
                loss_stage = criterion_stage(stages_pred, stages_true)
                loss_days = criterion_days(days_pred, days_true)
                total_loss = 0.7 * loss_stage + 0.3 * loss_days
                
                val_loss += total_loss.item()
                _, predicted = torch.max(stages_pred.data, 1)
                val_acc += (predicted == stages_true).sum().item() / len(stages_true)
        
        # 평균 계산
        train_loss /= len(train_loader)
        val_loss /= len(val_loader)
        train_acc /= len(train_loader)
        val_acc /= len(val_loader)
        
        scheduler.step()
        
        print(f"\n📊 Epoch {epoch+1} 결과:")
        print(f"   학습 - 손실: {train_loss:.4f}, 정확도: {train_acc:.3f}")
        print(f"   검증 - 손실: {val_loss:.4f}, 정확도: {val_acc:.3f}")
        
        # 최고 모델 저장
        if val_loss < best_val_loss:
            best_val_loss = val_loss
            torch.save({
                'model_state_dict': model.state_dict(),
                'train_loss': train_loss,
                'val_loss': val_loss,
                'train_acc': train_acc,
                'val_acc': val_acc
            }, 'best_multimodal_lettuce_model.pth')
            print("   ⭐ 새로운 최고 모델 저장!")
    
    print("✅ 멀티모달 학습 완료!")
    return model

# ============= 실시간 예측 함수 =============
def predict_multimodal(model_path, image_path, planting_date, photo_date=None):
    """이미지 + 날짜 정보로 종합 예측"""
    
    if photo_date is None:
        photo_date = datetime.now()
    
    # 1. 모델 로드
    model = MultimodalLettuceModel()
    checkpoint = torch.load(model_path, map_location='cpu')
    model.load_state_dict(checkpoint['model_state_dict'])
    model.eval()
    
    # 2. 이미지 전처리
    val_transform = transforms.Compose([
        transforms.Resize((224, 224)),
        transforms.ToTensor(),
        transforms.Normalize(mean=[0.485, 0.456, 0.406], std=[0.229, 0.224, 0.225])
    ])
    
    image = Image.open(image_path).convert("RGB")
    image_tensor = val_transform(image).unsqueeze(0)
    
    # 3. 날짜 정보 계산
    planting_date = pd.to_datetime(planting_date)
    photo_date = pd.to_datetime(photo_date)
    days_since_planting = (photo_date - planting_date).days
    days_to_harvest = max(0, 30 - days_since_planting)
    
    # 날짜 기반 예측 단계
    if days_since_planting <= 7:
        predicted_stage_by_date = 0
    elif days_since_planting <= 21:
        predicted_stage_by_date = 1
    else:
        predicted_stage_by_date = 2
    
    # 4. 메타데이터 생성
    metadata = torch.tensor([[
        days_since_planting / 35.0,
        days_to_harvest / 35.0,
        predicted_stage_by_date / 2.0,
        1.0  # 일치도는 예측 시 알 수 없으므로 1.0으로 설정
    ]], dtype=torch.float32)
    
    # 5. 예측 수행
    with torch.no_grad():
        stage_pred, days_pred, attention_weights = model(image_tensor, metadata)
        
        # 결과 해석
        stage_probs = F.softmax(stage_pred, dim=1)
        predicted_stage = torch.argmax(stage_probs, dim=1).item()
        stage_confidence = stage_probs[0][predicted_stage].item() * 100
        predicted_days = max(0, round(days_pred.item(), 1))
        
        vision_weight = attention_weights[0][0].item()
        metadata_weight = attention_weights[0][1].item()
        
        stage_names = ["🌱 정식기", "🌿 생육기", "🎯 수확기"]
        
        result = {
            "심은_날짜": planting_date.strftime("%Y-%m-%d"),
            "사진_날짜": photo_date.strftime("%Y-%m-%d"),
            "경과_일수": days_since_planting,
            "날짜_기반_예측": stage_names[predicted_stage_by_date],
            "AI_예측_단계": stage_names[predicted_stage],
            "예측_신뢰도": f"{stage_confidence:.1f}%",
            "수확_예상일": f"{predicted_days:.1f}일 후",
            "비전_가중치": f"{vision_weight:.2f}",
            "메타데이터_가중치": f"{metadata_weight:.2f}",
            "종합_판단": get_comprehensive_recommendation(
                days_since_planting, predicted_stage_by_date, predicted_stage, stage_confidence
            )
        }
        
        return result

def get_comprehensive_recommendation(days_elapsed, date_stage, ai_stage, confidence):
    """날짜와 AI 예측을 종합한 추천"""
    
    if abs(date_stage - ai_stage) >= 2:
        return "⚠️ 날짜와 AI 예측이 크게 다릅니다. 환경을 확인해주세요."
    elif confidence < 70:
        return "❓ 예측 신뢰도가 낮습니다. 다른 각도에서 재촬영해보세요."
    elif ai_stage == 2:
        return "🎉 수확 시기입니다!"
    elif days_elapsed > 25:
        return "🔔 수확 시기가 다가왔습니다."
    else:
        return "✅ 건강하게 성장 중입니다."

# ============= 메인 실행 =============
if __name__ == "__main__":
    print("🥬 멀티모달 스마트 상추 예측 시스템")
    print("=" * 50)
    
    if not os.path.exists(CSV_PATH):
        print(f"❌ CSV 파일을 찾을 수 없습니다: {CSV_PATH}")
    else:
        print(f"✅ CSV 파일 발견: {CSV_PATH}")
        
        # 학습 실행
        model = train_multimodal_model()
        
        print("\n🎉 학습 완료!")
        print("📁 생성된 파일: best_multimodal_lettuce_model.pth")
        
        # 테스트 예측 예시
        # result = predict_multimodal(
        #     'best_multimodal_lettuce_model.pth',
        #     'test_image.jpg',
        #     '2024-06-01',  # 심은 날짜
        #     '2024-06-15'   # 사진 날짜
        # )
        # print(f"\n🔍 예측 결과:")
        # for key, value in result.items():
        #     print(f"   {key}: {value}")