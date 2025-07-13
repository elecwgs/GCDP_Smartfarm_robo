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

# ============= 프로젝트 경로 설정 =============
PROJECT_ROOT = "C:/planttest0/"  # 실제 경로로 변경
CSV_PATH = os.path.join(PROJECT_ROOT, "0701", "dataset8_updated.csv")  # 업데이트된 CSV 사용
IMG_DIR = "C:/planttest0/TS/TS_3.8/TS_3.8"  # 이미지 폴더 경로

BATCH_SIZE = 16
EPOCHS = 5
DEVICE = torch.device("cuda" if torch.cuda.is_available() else "cpu")
TOTAL_GROWTH_DAYS = 30

# ============= CSV 데이터 분석 및 전처리 =============
def analyze_csv_data(csv_path):
    """CSV 데이터를 분석하고 전처리하는 함수"""
    print("📊 CSV 데이터 분석 중...")
    
    # CSV 파일 로드
    df = pd.read_csv(csv_path)
    print(f"📈 총 데이터 수: {len(df)}개")
    print(f"📋 컬럼들: {list(df.columns)}")
    
    # 기본 통계 정보
    print("\n📊 데이터 요약:")
    print(df.describe())
    
    # 결측값 확인
    missing_data = df.isnull().sum()
    if missing_data.any():
        print(f"\n⚠️ 결측값 발견:")
        print(missing_data[missing_data > 0])
    
    # 이미지 파일 존재 여부 확인
    if 'fname' in df.columns:
        missing_images = []
        for idx, fname in enumerate(df['fname']):
            img_path = os.path.join(IMG_DIR, fname)
            if not os.path.exists(img_path):
                missing_images.append(fname)
        
        if missing_images:
            print(f"\n❌ 없는 이미지 파일: {len(missing_images)}개")
            print("처음 5개:", missing_images[:5])
        else:
            print("✅ 모든 이미지 파일 존재 확인!")
    
    return df

def preprocess_data(df):
    """데이터 전처리 및 새로운 컬럼 생성"""
    print("🔧 데이터 전처리 중...")
    
    # 결측값 처리
    df = df.dropna()
    
    # 남은 일수 계산
    df["days_left"] = TOTAL_GROWTH_DAYS - df["estimated_days_elapsed"]
    df["days_left"] = df["days_left"].clip(lower=0)
    
    # 성장 단계 분류 (5단계)
    growth_stage_map = {
        "정식기": 0,
        "생육기": 1,
        "수확기": 2
    }
    df["growth_stage"] = df["growth_stage"].map(growth_stage_map)

    if df["growth_stage"].isnull().any():
        print("⚠️ 생육 단계에 정의되지 않은 값이 있습니다. 제거합니다.")
        df = df.dropna(subset=["growth_stage"])


    # 건강도 점수 계산 (임시 공식 - 나중에 실제 데이터로 개선)
    # 성장 속도와 예상 크기를 고려한 건강도
    df["health_score"] = np.clip(
        (df["estimated_days_elapsed"] / 30) * np.random.uniform(0.8, 1.0, len(df)), 
        0, 1
    )
    
    # 이상값 제거
    Q1 = df["days_left"].quantile(0.25)
    Q3 = df["days_left"].quantile(0.75)
    IQR = Q3 - Q1
    lower_bound = Q1 - 1.5 * IQR
    upper_bound = Q3 + 1.5 * IQR
    
    df = df[(df["days_left"] >= lower_bound) & (df["days_left"] <= upper_bound)]
    
    print(f"✅ 전처리 완료! 최종 데이터 수: {len(df)}개")
    print(f"📊 성장 단계 분포:")
    print(df["growth_stage"].value_counts().sort_index())
    
    return df

# ============= 향상된 데이터셋 클래스 =============
class SmartLettuceDataset(Dataset):
    """CSV 데이터를 활용한 스마트 상추 데이터셋"""
    
    def __init__(self, dataframe, img_dir, transform=None):
        self.data = dataframe.reset_index(drop=True)
        self.img_dir = img_dir
        self.transform = transform
        
        # 이미지 존재 여부 재확인
        valid_indices = []
        for idx in range(len(self.data)):
            fname = self.data.iloc[idx]['fname']
            img_path = os.path.join(img_dir, fname)
            if os.path.exists(img_path):
                valid_indices.append(idx)
        
        self.data = self.data.iloc[valid_indices].reset_index(drop=True)
        print(f"📷 유효한 이미지가 있는 데이터: {len(self.data)}개")

    def __len__(self):
        return len(self.data)

    def __getitem__(self, idx):
        row = self.data.iloc[idx]
        
        # 이미지 로드
        img_path = os.path.join(self.img_dir, row["fname"])
        try:
            image = Image.open(img_path).convert("RGB")
        except Exception as e:
            print(f"⚠️ 이미지 로드 오류: {row['fname']}")
            # 기본 녹색 이미지로 대체
            image = Image.new('RGB', (224, 224), color=(34, 139, 34))
        
        if self.transform:
            image = self.transform(image)
        
        # 라벨들
        days_left = torch.tensor([row["days_left"]], dtype=torch.float32)
        growth_stage = torch.tensor(row["growth_stage"], dtype=torch.long)
        health_score = torch.tensor([row["health_score"]], dtype=torch.float32)
        
        return image, days_left, growth_stage, health_score

# ============= 데이터 증강 =============
def get_transforms():
    """농업 환경에 특화된 데이터 증강"""
    
    train_transform = transforms.Compose([
        transforms.Resize((256, 256)),
        transforms.RandomCrop(224, padding=4),
        transforms.RandomHorizontalFlip(0.5),
        
        # 실외 농장 환경 시뮬레이션
        transforms.ColorJitter(
            brightness=0.3,    # 햇빛 변화
            contrast=0.3,      # 그림자 효과
            saturation=0.2,    # 계절별 색상 변화
            hue=0.1           # 약간의 색조 변화
        ),
        
        # 카메라 각도 변화
        transforms.RandomRotation(15),
        
        # 바람에 의한 흔들림 효과
        transforms.RandomApply([
            transforms.GaussianBlur(kernel_size=3, sigma=(0.1, 1.0))
        ], p=0.1),
        
        transforms.ToTensor(),
        transforms.Normalize(
            mean=[0.485, 0.456, 0.406], 
            std=[0.229, 0.224, 0.225]
        )
    ])
    
    val_transform = transforms.Compose([
        transforms.Resize((224, 224)),
        transforms.ToTensor(),
        transforms.Normalize(
            mean=[0.485, 0.456, 0.406], 
            std=[0.229, 0.224, 0.225]
        )
    ])
    
    return train_transform, val_transform

# ============= 라즈베리파이 최적화 모델 =============
class RaspberryPiLettuceModel(nn.Module):
    """라즈베리파이에 최적화된 상추 예측 모델"""
    
    def __init__(self, num_stages=5):
        super().__init__()
        
        # MobileNetV3 Small - 라즈베리파이 최적화
        self.backbone = models.mobilenet_v3_small(weights='DEFAULT')
        
        # 특징 추출 부분만 사용
        self.features = self.backbone.features
        self.avgpool = self.backbone.avgpool
        
        # 분류기 부분 교체
        self.classifier = nn.Sequential(
            nn.Linear(576, 256),  # MobileNetV3-small 출력 크기
            nn.Hardswish(),       # MobileNet에서 사용하는 활성화 함수
            nn.Dropout(0.2),
            nn.Linear(256, 128),
            nn.Hardswish(),
            nn.Dropout(0.1)
        )
        
        # 멀티태스크 헤드
        self.days_head = nn.Linear(128, 1)                    # 남은 일수
        self.stage_head = nn.Linear(128, num_stages)          # 성장 단계  
        self.health_head = nn.Sequential(                     # 건강도
            nn.Linear(128, 1),
            nn.Sigmoid()
        )

    def forward(self, x):
        # 특징 추출
        x = self.features(x)
        x = self.avgpool(x)
        x = torch.flatten(x, 1)
        
        # 공통 특징
        features = self.classifier(x)
        
        # 각 태스크별 예측
        days_left = self.days_head(features)
        growth_stage = self.stage_head(features)
        health_score = self.health_head(features)
        
        return days_left, growth_stage, health_score

# ============= 학습 및 검증 함수 =============
def train_with_validation():
    """검증 세트를 포함한 학습"""
    
    print("🚀 CSV 기반 상추 예측 모델 학습 시작!")
    
    # 1. 데이터 로드 및 분석
    df = analyze_csv_data(CSV_PATH)
    df = preprocess_data(df)
    
    # 2. 학습/검증 데이터 분할
    train_df, val_df = train_test_split(
        df, test_size=0.2, random_state=42, 
        stratify=df['growth_stage']  # 성장 단계별 비율 유지
    )
    
    print(f"📚 학습 데이터: {len(train_df)}개")
    print(f"🔍 검증 데이터: {len(val_df)}개")
    
    # 3. 데이터셋 및 로더 생성
    train_transform, val_transform = get_transforms()
    
    train_dataset = SmartLettuceDataset(train_df, IMG_DIR, train_transform)
    val_dataset = SmartLettuceDataset(val_df, IMG_DIR, val_transform)
    
    train_loader = DataLoader(train_dataset, batch_size=BATCH_SIZE, shuffle=True, num_workers=2)
    val_loader = DataLoader(val_dataset, batch_size=BATCH_SIZE, shuffle=False, num_workers=2)
    
    # 4. 모델 초기화
    model = RaspberryPiLettuceModel().to(DEVICE)
    
    # 파라미터 수 계산
    total_params = sum(p.numel() for p in model.parameters())
    trainable_params = sum(p.numel() for p in model.parameters() if p.requires_grad)
    print(f"🧠 전체 파라미터: {total_params:,}")
    print(f"🎯 학습 파라미터: {trainable_params:,}")
    
    # 5. 손실 함수 및 옵티마이저
    criterion_days = nn.MSELoss()
    criterion_stage = nn.CrossEntropyLoss()
    criterion_health = nn.L1Loss()
    
    optimizer = torch.optim.AdamW(model.parameters(), lr=1e-3, weight_decay=1e-4)
    scheduler = torch.optim.lr_scheduler.CosineAnnealingLR(optimizer, T_max=EPOCHS)
    
    # 6. 학습 기록
    train_losses = []
    val_losses = []
    best_val_loss = float('inf')
    
    # 7. 학습 루프
    for epoch in range(EPOCHS):
        # 학습
        model.train()
        train_loss = 0
        train_acc = 0
        
        pbar = tqdm(train_loader, desc=f"🌱 학습 {epoch+1}/{EPOCHS}")
        for images, days_true, stages_true, health_true in pbar:
            images = images.to(DEVICE)
            days_true = days_true.to(DEVICE)
            stages_true = stages_true.to(DEVICE)
            health_true = health_true.to(DEVICE)
            
            # 순전파
            days_pred, stages_pred, health_pred = model(images)
            
            # 손실 계산
            loss_days = criterion_days(days_pred, days_true)
            loss_stage = criterion_stage(stages_pred, stages_true)
            loss_health = criterion_health(health_pred, health_true)
            
            total_loss = 0.5 * loss_days + 0.3 * loss_stage + 0.2 * loss_health
            
            # 역전파
            optimizer.zero_grad()
            total_loss.backward()
            optimizer.step()
            
            # 통계
            train_loss += total_loss.item()
            _, predicted = torch.max(stages_pred.data, 1)
            train_acc += (predicted == stages_true).sum().item() / len(stages_true)
            
            pbar.set_postfix({
                'Loss': f"{train_loss/(len(pbar)):.3f}",
                'Acc': f"{train_acc/(len(pbar)):.3f}"
            })
        
        # 검증
        model.eval()
        val_loss = 0
        val_acc = 0
        
        with torch.no_grad():
            for images, days_true, stages_true, health_true in val_loader:
                images = images.to(DEVICE)
                days_true = days_true.to(DEVICE)
                stages_true = stages_true.to(DEVICE)
                health_true = health_true.to(DEVICE)
                
                days_pred, stages_pred, health_pred = model(images)
                
                loss_days = criterion_days(days_pred, days_true)
                loss_stage = criterion_stage(stages_pred, stages_true)
                loss_health = criterion_health(health_pred, health_true)
                
                total_loss = 0.5 * loss_days + 0.3 * loss_stage + 0.2 * loss_health
                val_loss += total_loss.item()
                
                _, predicted = torch.max(stages_pred.data, 1)
                val_acc += (predicted == stages_true).sum().item() / len(stages_true)
        
        # 평균 계산
        train_loss /= len(train_loader)
        val_loss /= len(val_loader)
        train_acc /= len(train_loader)
        val_acc /= len(val_loader)
        
        train_losses.append(train_loss)
        val_losses.append(val_loss)
        
        # 스케줄러 업데이트
        scheduler.step()
        
        # 결과 출력
        print(f"\n📊 Epoch {epoch+1} 결과:")
        print(f"   학습 - 손실: {train_loss:.4f}, 정확도: {train_acc:.3f}")
        print(f"   검증 - 손실: {val_loss:.4f}, 정확도: {val_acc:.3f}")
        print(f"   학습률: {optimizer.param_groups[0]['lr']:.6f}")
        
        # 최고 모델 저장
        if val_loss < best_val_loss:
            best_val_loss = val_loss
            torch.save({
                'epoch': epoch,
                'model_state_dict': model.state_dict(),
                'optimizer_state_dict': optimizer.state_dict(),
                'train_loss': train_loss,
                'val_loss': val_loss,
                'train_acc': train_acc,
                'val_acc': val_acc
            }, 'best_lettuce_model.pth')
            print("   ⭐ 새로운 최고 모델 저장!")
    
    print("✅ 학습 완료!")
    return model

# ============= 실시간 예측 함수 =============
def predict_from_raspberry_pi(model_path, image_path):
    """라즈베리파이에서 실시간 예측"""
    
    # 모델 로드
    model = RaspberryPiLettuceModel()
    checkpoint = torch.load(model_path, map_location='cpu')
    model.load_state_dict(checkpoint['model_state_dict'])
    model.eval()
    
    # 이미지 전처리
    _, val_transform = get_transforms()
    image = Image.open(image_path).convert("RGB")
    image_tensor = val_transform(image).unsqueeze(0)
    
    # 예측
    with torch.no_grad():
        days_pred, stage_pred, health_pred = model(image_tensor)
        
        # 결과 해석
        days_left = max(0, round(days_pred.item(), 1))
        stage_probs = F.softmax(stage_pred, dim=1)
        stage_idx = torch.argmax(stage_probs, dim=1).item()
        stage_conf = stage_probs[0][stage_idx].item() * 100
        health_score = health_pred.item() * 100
        
        stage_names = ["🌱 새싹", "🌿 초기", "🥬 중기", "🍃 후기", "🎯 수확"]
        
        result = {
            "수확_예상일": f"{days_left}일 후",
            "현재_단계": stage_names[stage_idx],
            "단계_신뢰도": f"{stage_conf:.1f}%",
            "건강도": f"{health_score:.1f}점",
            "추천_조치": get_recommendation(days_left, stage_idx, health_score)
        }
        
        return result

def get_recommendation(days_left, stage_idx, health_score):
    """상황별 추천 조치"""
    
    if health_score < 60:
        return "⚠️ 건강도가 낮습니다. 영양분 공급을 확인하세요."
    elif stage_idx >= 4:
        return "🎉 수확 시기입니다!"
    elif days_left <= 3:
        return "🔔 곧 수확 가능합니다. 준비하세요!"
    elif days_left <= 7:
        return "⏰ 1주일 내 수확 예정입니다."
    else:
        return "✅ 건강하게 자라고 있습니다."

# ============= 메인 실행 =============
if __name__ == "__main__":
    print("🥬 CSV 기반 스마트 상추 예측 시스템")
    print("=" * 50)
    
    # CSV 파일 경로 확인
    if not os.path.exists(CSV_PATH):
        print(f"❌ CSV 파일을 찾을 수 없습니다: {CSV_PATH}")
        print("📁 경로를 확인해주세요!")
    else:
        print(f"✅ CSV 파일 발견: {CSV_PATH}")
        
        # 학습 실행
        model = train_with_validation()
        
        print("\n🎉 모든 과정 완료!")
        print("📁 생성된 파일: best_lettuce_model.pth")
        
        # 테스트 예측 예시
        # result = predict_from_raspberry_pi('best_lettuce_model.pth', 'test_image.jpg')
        # print(f"\n🔍 예측 결과: {result}")
