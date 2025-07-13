import os
import json
import shutil
import re
from pathlib import Path

# 경로 설정
JSON_SOURCE_PATH = 'C:/planttest0/0701/getplant'  # JSON 파일들이 있는 경로
TS_SOURCE_PATH = 'C:/planttest0/TS'  # TS 폴더 경로
TARGET_PATH = './0701/getplant_image'  # 매칭된 이미지를 저장할 경로

# 타겟 디렉토리 생성
os.makedirs(TARGET_PATH, exist_ok=True)

def find_all_jpg_files(dir_path):
    """TS 폴더에서 모든 JPG 파일 찾기"""
    jpg_files = []
    for root, dirs, files in os.walk(dir_path):
        for file in files:
            if file.lower().endswith('.jpg'):
                jpg_files.append(os.path.join(root, file))
    return jpg_files

def extract_image_id_from_filename(filename):
    """파일명에서 마지막 숫자 추출 (image_id와 매칭용)"""
    # 파일명에서 마지막 숫자 시퀀스를 추출
    # 예: C11_L01_01_005_207927.jpg -> 207927
    name_without_ext = os.path.splitext(filename)[0]  # 확장자 제거
    numbers = re.findall(r'\d+', name_without_ext)
    if numbers:
        # 마지막 숫자를 image_id로 간주
        return int(numbers[-1])
    return None

def process_image_matching():
    """메인 처리 함수"""
    try:
        # 1. JSON 파일들 읽기
        json_files = [f for f in os.listdir(JSON_SOURCE_PATH) if f.endswith('.json')]
        print(f"Found {len(json_files)} JSON files")
        
        # 2. TS 폴더에서 모든 JPG 파일 찾기
        print('Searching for JPG files in TS folder...')
        all_jpg_files = find_all_jpg_files(TS_SOURCE_PATH)
        print(f"Found {len(all_jpg_files)} JPG files")
        
        # 3. 파일명으로 image_id 매핑 생성
        image_id_to_file_map = {}
        for file_path in all_jpg_files:
            filename = os.path.basename(file_path)
            image_id = extract_image_id_from_filename(filename)
            if image_id:
                image_id_to_file_map[image_id] = file_path
        
        print(f"Created mapping for {len(image_id_to_file_map)} images")
        
        # 4. 각 JSON 파일 처리
        matched_count = 0
        not_found_count = 0
        
        for json_file in json_files:
            json_path = os.path.join(JSON_SOURCE_PATH, json_file)
            
            try:
                with open(json_path, 'r', encoding='utf-8') as f:
                    json_data = json.load(f)
                
                if 'images' in json_data and 'image_id' in json_data['images']:
                    image_id = json_data['images']['image_id']
                    fname = json_data['images'].get('fname', '')
                    
                    # fname에서도 마지막 숫자 추출하여 검증
                    if fname:
                        fname_image_id = extract_image_id_from_filename(fname)
                        if fname_image_id != image_id:
                            print(f"⚠ Warning: image_id ({image_id}) doesn't match fname number ({fname_image_id}) in {json_file}")
                    
                    matched_image_path = image_id_to_file_map.get(image_id)
                    
                    if matched_image_path:
                        # 매칭된 이미지 복사
                        target_filename = os.path.basename(matched_image_path)
                        target_file_path = os.path.join(TARGET_PATH, target_filename)
                        
                        shutil.copy2(matched_image_path, target_file_path)
                        print(f"✓ Matched: {json_file} -> {target_filename} (ID: {image_id}, fname: {fname})")
                        matched_count += 1
                    else:
                        print(f"✗ Not found: {json_file} (ID: {image_id}, fname: {fname})")
                        not_found_count += 1
                else:
                    print(f"✗ Invalid JSON structure: {json_file}")
                    not_found_count += 1
                    
            except Exception as e:
                print(f"✗ Error processing {json_file}: {e}")
                not_found_count += 1
        
        print('\n=== Processing Complete ===')
        print(f"Total JSON files: {len(json_files)}")
        print(f"Matched and copied: {matched_count}")
        print(f"Not found: {not_found_count}")
        print(f"Target directory: {TARGET_PATH}")
        
    except Exception as error:
        print(f'Error during processing: {error}')

# 실행
if __name__ == "__main__":
    process_image_matching()