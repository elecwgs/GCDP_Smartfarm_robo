import os
import json
import shutil
from collections import defaultdict

# 경로 설정
TL_BASE_DIR = "C:/planttest0/TL"
TS_BASE_DIR = "C:/planttest0/TS"
GETPLANT_DIR = "C:/planttest0/0701/getplant"

# 복사 대상 폴더가 없으면 생성
os.makedirs(GETPLANT_DIR, exist_ok=True)

growth_keywords = {
    "정식기": "정식기",
    "생육기": "생육기",
    "수확기": "수확기"
}

for tl_folder in os.listdir(TL_BASE_DIR):
    tl_path = os.path.join(TL_BASE_DIR, tl_folder)
    if not os.path.isdir(tl_path):
        continue

    ts_folder_name = tl_folder.replace("TL_", "TS_")
    ts_path = os.path.join(TS_BASE_DIR, ts_folder_name)

    if not os.path.exists(ts_path):
        print(f"🔹 TS 폴더 없음: {tl_folder}, 스킵")
        continue

    print(f"📂 분석 중: {tl_folder}")

    for filename in os.listdir(tl_path):
        if not filename.endswith(".json"):
            continue

        json_path = os.path.join(tl_path, filename)

        try:
            with open(json_path, "r", encoding="utf-8") as f:
                data = json.load(f)

            stage = data.get("images", {}).get("growth_stage", None)

            if stage == "수확기":
                target_path = os.path.join(GETPLANT_DIR, filename)
                shutil.copy2(json_path, target_path)
                print(f"✅ 수확기 복사: {filename}")

        except Exception as e:
            print(f"❌ JSON 에러: {filename}, {e}")
