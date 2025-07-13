#각 폴더 검사해서 생육단계 데이터 확인
import os
import json
from collections import defaultdict

# 경로 설정
TL_BASE_DIR = "C:/planttest0/TL"
TS_BASE_DIR = "C:/planttest0/TS"

growth_keywords = {
    "정식기": "정식기",
    "생육기": "생육기",
    "수확기": "수확기"
}

all_counts = {}

for tl_folder in os.listdir(TL_BASE_DIR):
    tl_path = os.path.join(TL_BASE_DIR, tl_folder)
    if not os.path.isdir(tl_path):
        continue  # 폴더가 아닌 경우 스킵

    # 대응되는 TS 폴더 이름 만들기 (예: TL_3.상추6 → TS_3.상추6)
    ts_folder_name = tl_folder.replace("TL_", "TS_")
    ts_path = os.path.join(TS_BASE_DIR, ts_folder_name)

    if not os.path.exists(ts_path):
        print(f"🔹 TS 폴더 없음: {tl_folder}, 스킵")
        continue

    print(f"📂 분석 중: {tl_folder}")
    count = defaultdict(int)

    for filename in os.listdir(tl_path):
        if not filename.endswith(".json"):
            continue

        json_path = os.path.join(tl_path, filename)

        try:
            with open(json_path, "r", encoding="utf-8") as f:
                data = json.load(f)

            stage = data.get("images", {}).get("growth_stage", None)

            if stage in growth_keywords:
                count[stage] += 1
            else:
                print(f"⚠️ 미지정 stage: {filename}")

        except Exception as e:
            print(f"❌ JSON 에러: {filename}, {e}")

    all_counts[tl_folder] = dict(count)

# 결과 출력
print("\n\n🌱 전체 결과:")
for folder, counts in all_counts.items():
    print(f"\n📁 {folder}")
    for stage_name in ["정식기", "생육기", "수확기"]:
        print(f"  - {stage_name}: {counts.get(stage_name, 0)}개")
