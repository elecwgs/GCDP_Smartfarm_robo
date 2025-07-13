import pandas as pd
from datetime import datetime


input_csv = r"C:/planttest0/0701/dataset8_3.3_raw.csv"      # 원본 CSV
output_csv = r"C:/planttest0/0701/dataset8_3.3_updated.csv"  # 저장될 경로

# CSV 불러오기
df = pd.read_csv(input_csv)

# 날짜 컬럼이 문자열이면 datetime으로 변환
if not pd.api.types.is_datetime64_any_dtype(df["date_captured"]):
    df["date_captured"] = pd.to_datetime(df["date_captured"], errors='coerce')

# 변환 안 된 row가 있다면 경고
if df["date_captured"].isnull().any():
    print("⚠️ 날짜 파싱 실패한 행 있음 → NaT 표시됨. 날짜 포맷을 확인하세요.")
    print(df[df["date_captured"].isnull()])

# 가장 오래된 날짜 찾기 (가장 먼저 찍은 날짜)
earliest_date = df["date_captured"].min()
print(f"📅 기준 날짜 (최초 촬영일): {earliest_date.strftime('%Y-%m-%d')}")

# 경과일 계산
df["estimated_days_elapsed"] = (df["date_captured"] - earliest_date).dt.days

# 확인용 출력
print(df[["fname", "date_captured", "estimated_days_elapsed"]].head())

# 저장
df.to_csv(output_csv, index=False)
print(f"\n✅ 완료: {output_csv}에 저장됨")
