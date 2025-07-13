import os
import json
import glob
import shutil
import pandas as pd

# === 경로 설정 ===
TS_BASE_DIR = r"C:\planttest0\TS"
GETPLANT_JSON_DIR = r"C:\planttest0\0701\getplant"
GETPLANT_IMAGE_DIR = r"C:\planttest0\0701\getplant_image"
CSV_OUTPUT_DIR = r"C:\planttest0"

os.makedirs(GETPLANT_IMAGE_DIR, exist_ok=True)

matched_records = []
unmatched_records = []

# === getplant의 JSON 파일 순회 ===
for json_file in os.listdir(GETPLANT_JSON_DIR):
    if not json_file.endswith(".json"):
        continue

    json_path = os.path.join(GETPLANT_JSON_DIR, json_file)

    try:
        with open(json_path, "r", encoding="utf-8") as f:
            data = json.load(f)

        image_id = str(data["images"]["image_id"])
        fname = data["images"]["fname"]
        growth_stage = data["images"].get("growth_stage", "")
        date_captured = data["images"].get("date_captured", "")

        found = False

        # === TS 전체 폴더 순회하며 이미지 찾기 ===
        for ts_folder in os.listdir(TS_BASE_DIR):
            ts_inner_path = os.path.join(TS_BASE_DIR, ts_folder, ts_folder)

            if not os.path.isdir(ts_inner_path):
                continue

            matched = glob.glob(os.path.join(ts_inner_path, f"*{image_id}*.jpg"))
            if matched:
                src = matched[0]
                dst = os.path.join(GETPLANT_IMAGE_DIR, os.path.basename(src))
                shutil.copy2(src, dst)
                print(f"✅ 복사 완료: {os.path.basename(src)} (from {ts_folder})")

                matched_records.append({
                    "json_file": json_file,
                    "image_file": os.path.basename(src),
                    "image_id": image_id,
                    "ts_folder": ts_folder,
                    "growth_stage": growth_stage,
                    "date_captured": date_captured
                })
                found = True
                break

        if not found:
            print(f"⚠️ 이미지 없음: image_id={image_id}")
            unmatched_records.append({
                "json_file": json_file,
                "image_id": image_id
            })

    except Exception as e:
        print(f"❌ 오류 발생: {json_file}, {e}")
        unmatched_records.append({
            "json_file": json_file,
            "image_id": "ERROR"
        })

# === CSV 저장 ===
matched_df = pd.DataFrame(matched_records)
unmatched_df = pd.DataFrame(unmatched_records)

matched_csv_path = os.path.join(CSV_OUTPUT_DIR, "matched_list.csv")
unmatched_csv_path = os.path.join(CSV_OUTPUT_DIR, "unmatched_list.csv")

matched_df.to_csv(matched_csv_path, index=False)
unmatched_df.to_csv(unmatched_csv_path, index=False)

print(f"\n📄 매칭된 이미지 목록 저장 완료: {matched_csv_path} ({len(matched_df)}개)")
print(f"📄 미매칭 JSON 목록 저장 완료: {unmatched_csv_path} ({len(unmatched_df)}개)")
