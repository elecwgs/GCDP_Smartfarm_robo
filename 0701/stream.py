import streamlit as st
import torch
from PIL import Image
import pandas as pd
import os
import torch.nn.functional as F
from torchvision import transforms
import numpy as np
from datetime import datetime, timedelta
import sys

# 멀티모달 모델 import (train.py에서 가져오기)
from train import MultimodalLettuceModel, analyze_multimodal_data, predict_multimodal

#  설정
MODEL_PATH = "best_multimodal_lettuce_model.pth"
CSV_PATH = "C:/planttest0/0701/dataset8_3.3_updated.csv"
IMG_DIR = "C:/planttest0/TS/TS_3.3/TS_3.3"
DEVICE = torch.device("cuda" if torch.cuda.is_available() else "cpu")

#  라벨 매핑
stage_names = ["🌱 정식기", "🌿 생육기", "🎯 수확기"]

# 페이지 설정
st.set_page_config(
    page_title="멀티모달 상추 생육 예측 대시보드",
    page_icon="🥬",
    layout="wide"
)

# ✅ 제목
st.title("🥬 멀티모달 상추 생육 예측 대시보드")
st.markdown("---")

# ✅ 사이드바 - 모드 선택
st.sidebar.header("📋 메뉴")
mode = st.sidebar.selectbox(
    "모드를 선택하세요",
    ["📊 데이터셋 분석", "🔍 개별 예측", "📈 실시간 예측"]
)

# ✅ 공통 함수들
@st.cache_data
def load_data():
    """데이터 로드 및 전처리"""
    if not os.path.exists(CSV_PATH):
        st.error(f"❌ CSV 파일을 찾을 수 없습니다: {CSV_PATH}")
        return None
    
    df = analyze_multimodal_data(CSV_PATH)
    return df

@st.cache_resource
def load_model():
    """모델 로드"""
    if not os.path.exists(MODEL_PATH):
        st.error(f"❌ 모델 파일을 찾을 수 없습니다: {MODEL_PATH}")
        st.info("먼저 train.py를 실행하여 모델을 학습해주세요.")
        return None
    
    model = MultimodalLettuceModel()
    checkpoint = torch.load(MODEL_PATH, map_location=DEVICE)
    model.load_state_dict(checkpoint['model_state_dict'])
    model.to(DEVICE)
    model.eval()
    return model, checkpoint

def get_val_transform():
    """검증용 이미지 변환"""
    return transforms.Compose([
        transforms.Resize((224, 224)),
        transforms.ToTensor(),
        transforms.Normalize(mean=[0.485, 0.456, 0.406], std=[0.229, 0.224, 0.225])
    ])

def predict_single_image(model, image_path, metadata_features):
    """단일 이미지 예측"""
    try:
        # 이미지 로드 및 전처리
        image = Image.open(image_path).convert("RGB")
        transform = get_val_transform()
        image_tensor = transform(image).unsqueeze(0).to(DEVICE)
        
        # 메타데이터 텐서 생성
        metadata_tensor = torch.tensor([metadata_features], dtype=torch.float32).to(DEVICE)
        
        # 예측 수행
        with torch.no_grad():
            stage_pred, days_pred, attention_weights = model(image_tensor, metadata_tensor)
            
            # 결과 해석
            stage_probs = F.softmax(stage_pred, dim=1)
            predicted_stage = torch.argmax(stage_probs, dim=1).item()
            stage_confidence = stage_probs[0][predicted_stage].item() * 100
            predicted_days = max(0, round(days_pred.item(), 1))
            
            vision_weight = attention_weights[0][0].item()
            metadata_weight = attention_weights[0][1].item()
            
            return {
                "predicted_stage": predicted_stage,
                "stage_confidence": stage_confidence,
                "predicted_days": predicted_days,
                "vision_weight": vision_weight,
                "metadata_weight": metadata_weight,
                "stage_probs": stage_probs[0].cpu().numpy()
            }
    
    except Exception as e:
        st.error(f"예측 중 오류 발생: {str(e)}")
        return None

# ✅ 메인 콘텐츠
if mode == "📊 데이터셋 분석":
    st.header("📊 데이터셋 분석")
    
    # 데이터 로드
    df = load_data()
    if df is not None:
        # 기본 통계
        col1, col2, col3, col4 = st.columns(4)
        with col1:
            st.metric("전체 데이터", f"{len(df):,}개")
        with col2:
            consistency_rate = df['stage_consistency'].mean()
            st.metric("날짜-라벨 일치도", f"{consistency_rate:.1%}")
        with col3:
            avg_days = df['days_since_planting'].mean()
            st.metric("평균 경과일", f"{avg_days:.1f}일")
        with col4:
            avg_harvest_days = df['days_to_harvest'].mean()
            st.metric("평균 수확 예상일", f"{avg_harvest_days:.1f}일")
        
        # 성장 단계별 분포
        st.subheader("🌱 성장 단계별 분포")
        stage_counts = df['actual_stage'].value_counts().sort_index()
        stage_data = pd.DataFrame({
            '단계': [stage_names[i] for i in stage_counts.index],
            '개수': stage_counts.values
        })
        st.bar_chart(stage_data.set_index('단계'))
        
        # 경과일 분포
        st.subheader("📅 경과일 분포")
        st.histogram_chart(df['days_since_planting'])
        
        # 데이터 테이블
        st.subheader("📋 데이터 미리보기")
        display_df = df[['fname', 'growth_stage', 'days_since_planting', 'days_to_harvest', 
                        'predicted_stage_name', 'stage_consistency']].head(10)
        st.dataframe(display_df)

elif mode == "🔍 개별 예측":
    st.header("🔍 개별 이미지 예측")
    
    # 데이터 및 모델 로드
    df = load_data()
    model_info = load_model()
    
    if df is not None and model_info is not None:
        model, checkpoint = model_info
        
        # 이미지 선택
        st.subheader("📷 이미지 선택")
        selected_fname = st.selectbox(
            "분석할 이미지를 선택하세요",
            df['fname'].tolist(),
            key="image_selector"
        )
        
        if selected_fname:
            # 선택된 행 데이터
            row = df[df["fname"] == selected_fname].iloc[0]
            img_path = os.path.join(IMG_DIR, selected_fname)
            
            if os.path.exists(img_path):
                # 이미지 표시
                image = Image.open(img_path).convert("RGB")
                
                col1, col2 = st.columns([1, 1])
                
                with col1:
                    st.image(image, caption=f"선택한 이미지: {selected_fname}", use_column_width=True)
                
                with col2:
                    # 실제 정보
                    st.subheader("📋 실제 정보")
                    st.write(f"**생육 단계**: {row['growth_stage']}")
                    st.write(f"**경과일**: {row['days_since_planting']}일")
                    st.write(f"**수확까지**: {row['days_to_harvest']}일")
                    st.write(f"**날짜 예측**: {row['predicted_stage_name']}")
                    st.write(f"**일치도**: {'✅' if row['stage_consistency'] else '❌'}")
                
                # 예측 수행
                st.subheader("🧠 AI 예측 결과")
                
                # 메타데이터 특징 생성
                metadata_features = [
                    row['days_since_planting'] / 35.0,
                    row['days_to_harvest'] / 35.0,
                    row['predicted_stage_by_date'] / 2.0,
                    row['stage_consistency']
                ]
                
                prediction = predict_single_image(model, img_path, metadata_features)
                
                if prediction:
                    col1, col2, col3 = st.columns(3)
                    
                    with col1:
                        st.metric(
                            "예측 단계",
                            stage_names[prediction['predicted_stage']],
                            f"{prediction['stage_confidence']:.1f}% 신뢰도"
                        )
                    
                    with col2:
                        st.metric(
                            "수확 예상일",
                            f"{prediction['predicted_days']:.1f}일",
                            f"실제 대비 {prediction['predicted_days'] - row['days_to_harvest']:.1f}일"
                        )
                    
                    with col3:
                        st.metric(
                            "어텐션 가중치",
                            f"비전: {prediction['vision_weight']:.2f}",
                            f"메타: {prediction['metadata_weight']:.2f}"
                        )
                    
                    # 각 단계별 확률
                    st.subheader("📊 단계별 예측 확률")
                    prob_data = pd.DataFrame({
                        '단계': stage_names,
                        '확률': prediction['stage_probs'] * 100
                    })
                    st.bar_chart(prob_data.set_index('단계'))
                    
                    # 모델 성능 정보
                    st.subheader("📈 모델 성능")
                    perf_col1, perf_col2 = st.columns(2)
                    with perf_col1:
                        st.metric("학습 정확도", f"{checkpoint.get('train_acc', 0):.1%}")
                        st.metric("학습 손실", f"{checkpoint.get('train_loss', 0):.4f}")
                    with perf_col2:
                        st.metric("검증 정확도", f"{checkpoint.get('val_acc', 0):.1%}")
                        st.metric("검증 손실", f"{checkpoint.get('val_loss', 0):.4f}")
            
            else:
                st.error(f"❌ 이미지 파일을 찾을 수 없습니다: {img_path}")

elif mode == "📈 실시간 예측":
    st.header("📈 실시간 예측")
    
    # 파일 업로드
    uploaded_file = st.file_uploader(
        "상추 이미지를 업로드하세요",
        type=['jpg', 'jpeg', 'png'],
        help="상추의 전체적인 모습이 잘 보이는 이미지를 업로드해주세요"
    )
    
    if uploaded_file is not None:
        # 이미지 표시
        image = Image.open(uploaded_file).convert("RGB")
        st.image(image, caption="업로드된 이미지", use_column_width=True)
        
        # 날짜 입력
        col1, col2 = st.columns(2)
        with col1:
            planting_date = st.date_input(
                "심은 날짜를 선택하세요",
                value=datetime.now() - timedelta(days=14),
                max_value=datetime.now().date()
            )
        
        with col2:
            photo_date = st.date_input(
                "사진 촬영 날짜",
                value=datetime.now().date(),
                max_value=datetime.now().date()
            )
        
        # 예측 버튼
        if st.button("🔍 예측하기", type="primary"):
            try:
                # 임시 파일 저장
                temp_path = "temp_upload.jpg"
                image.save(temp_path)
                
                # 예측 수행
                result = predict_multimodal(
                    MODEL_PATH,
                    temp_path,
                    planting_date.strftime("%Y-%m-%d"),
                    photo_date.strftime("%Y-%m-%d")
                )
                
                # 결과 표시
                st.subheader("🎯 예측 결과")
                
                col1, col2, col3 = st.columns(3)
                with col1:
                    st.metric("AI 예측 단계", result["AI_예측_단계"])
                    st.metric("날짜 기반 예측", result["날짜_기반_예측"])
                
                with col2:
                    st.metric("예측 신뢰도", result["예측_신뢰도"])
                    st.metric("수확 예상일", result["수확_예상일"])
                
                with col3:
                    st.metric("경과일", f"{result['경과_일수']}일")
                    st.metric("비전 가중치", f"{float(result['비전_가중치']):.2f}")
                
                # 종합 판단
                st.subheader("💡 종합 판단")
                judgment = result["종합_판단"]
                if "수확" in judgment:
                    st.success(judgment)
                elif "⚠️" in judgment or "❓" in judgment:
                    st.warning(judgment)
                else:
                    st.info(judgment)
                
                # 상세 정보
                with st.expander("📋 상세 정보"):
                    for key, value in result.items():
                        st.write(f"**{key}**: {value}")
                
                # 임시 파일 삭제
                if os.path.exists(temp_path):
                    os.remove(temp_path)
                    
            except Exception as e:
                st.error(f"예측 중 오류가 발생했습니다: {str(e)}")
                st.info("모델 파일이 존재하는지 확인하고, train.py를 먼저 실행해주세요.")

# ✅ 사이드바 - 추가 정보
st.sidebar.markdown("---")
st.sidebar.subheader("📚 도움말")
st.sidebar.markdown("""
**모드 설명:**
- **데이터셋 분석**: 전체 데이터 통계 확인
- **개별 예측**: 기존 데이터로 예측 테스트
- **실시간 예측**: 새로운 이미지 업로드하여 예측

**주의사항:**
- 모델이 학습되어야 예측이 가능합니다
- 이미지는 상추 전체가 잘 보이게 촬영해주세요
- 날짜 정보가 정확해야 더 정확한 예측이 가능합니다
""")

st.sidebar.markdown("---")
st.sidebar.info("🔧 **개발자**: 멀티모달 AI 시스템 v1.0")