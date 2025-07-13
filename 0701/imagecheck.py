import os
import cv2

folder = "C:/planttest0/TS/TS_3.8/TS_3.8"
for fname in os.listdir(folder):
    if not fname.endswith(".jpg"):
        continue
    path = os.path.join(folder, fname)
    img = cv2.imread(path)
    h, w = img.shape[:2]
    print(f"{fname}: {w} x {h}")
print("✅ cropdata.py 실행됨!")

# 예시: 기본 폴더에 jpg 이미지 있는지 확인
import os
folder_path = "C:/planttest0/TS/extracted/TS_3.상추3"
files = [f for f in os.listdir(folder_path) if f.lower().endswith(".jpg")]

print(f"총 {len(files)}개 이미지가 있습니다.")
print("샘플 파일 10개:")
for f in files[:10]:
    print("-", f)
