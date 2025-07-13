import os
import json
import shutil
from pathlib import Path

# 경로 설정
JSON_SOURCE_PATH = 'C:/planttest0/0701/getplant'  # JSON 파일들이 있는 경로
IMAGE_SOURCE_PATH = 'C:/planttest0/0701/getplant_image'  # 이미지 파일들이 있는 경로
OUTPUT_BASE_PATH = './0701/classified'  # 분류된 파일들을 저장할 기본 경로

# 분류할 kind_type 목록
KIND_TYPES = ['로메인', '햇살적축면']

def create_output_directories():
    """출력 디렉토리 생성"""
    for kind_type in KIND_TYPES:
        # 각 kind_type별로 json과 image 폴더 생성
        json_dir = os.path.join(OUTPUT_BASE_PATH, kind_type, 'json')
        image_dir = os.path.join(OUTPUT_BASE_PATH, kind_type, 'image')
        os.makedirs(json_dir, exist_ok=True)
        os.makedirs(image_dir, exist_ok=True)
        print(f"Created directories for {kind_type}")

def extract_image_id_from_filename(filename):
    """파일명에서 마지막 숫자 추출"""
    import re
    name_without_ext = os.path.splitext(filename)[0]
    numbers = re.findall(r'\d+', name_without_ext)
    if numbers:
        return int(numbers[-1])
    return None

def get_image_files_mapping():
    """이미지 파일들의 image_id 매핑 생성"""
    image_mapping = {}
    if os.path.exists(IMAGE_SOURCE_PATH):
        for filename in os.listdir(IMAGE_SOURCE_PATH):
            if filename.lower().endswith('.jpg'):
                image_id = extract_image_id_from_filename(filename)
                if image_id:
                    image_mapping[image_id] = os.path.join(IMAGE_SOURCE_PATH, filename)
    return image_mapping

def classify_files():
    """파일들을 kind_type별로 분류"""
    try:
        # 출력 디렉토리 생성
        create_output_directories()
        
        # 이미지 파일 매핑 생성
        image_mapping = get_image_files_mapping()
        print(f"Found {len(image_mapping)} image files")
        
        # JSON 파일들 읽기
        json_files = [f for f in os.listdir(JSON_SOURCE_PATH) if f.endswith('.json')]
        print(f"Found {len(json_files)} JSON files")
        
        # 통계 변수
        classification_stats = {kind_type: {'json': 0, 'image': 0} for kind_type in KIND_TYPES}
        unclassified_count = 0
        error_count = 0
        
        for json_file in json_files:
            json_path = os.path.join(JSON_SOURCE_PATH, json_file)
            
            try:
                with open(json_path, 'r', encoding='utf-8') as f:
                    json_data = json.load(f)
                
                # kind_type과 image_id 추출
                if 'images' in json_data and 'kind_type' in json_data['images']:
                    kind_type = json_data['images']['kind_type']
                    image_id = json_data['images'].get('image_id')
                    
                    if kind_type in KIND_TYPES:
                        # JSON 파일 복사
                        target_json_dir = os.path.join(OUTPUT_BASE_PATH, kind_type, 'json')
                        target_json_path = os.path.join(target_json_dir, json_file)
                        shutil.copy2(json_path, target_json_path)
                        classification_stats[kind_type]['json'] += 1
                        
                        # 해당하는 이미지 파일 복사
                        if image_id and image_id in image_mapping:
                            source_image_path = image_mapping[image_id]
                            image_filename = os.path.basename(source_image_path)
                            target_image_dir = os.path.join(OUTPUT_BASE_PATH, kind_type, 'image')
                            target_image_path = os.path.join(target_image_dir, image_filename)
                            shutil.copy2(source_image_path, target_image_path)
                            classification_stats[kind_type]['image'] += 1
                            
                            print(f"✓ Classified: {json_file} -> {kind_type} (ID: {image_id}, Image: {image_filename})")
                        else:
                            print(f"⚠ JSON classified but image not found: {json_file} (ID: {image_id}, Type: {kind_type})")
                    else:
                        print(f"⚠ Unknown kind_type: {kind_type} in {json_file}")
                        unclassified_count += 1
                else:
                    print(f"✗ Invalid JSON structure (missing kind_type): {json_file}")
                    unclassified_count += 1
                    
            except Exception as e:
                print(f"✗ Error processing {json_file}: {e}")
                error_count += 1
        
        # 결과 출력
        print('\n=== Classification Complete ===')
        print(f"Total JSON files processed: {len(json_files)}")
        print(f"Total image files available: {len(image_mapping)}")
        print()
        
        for kind_type in KIND_TYPES:
            print(f"{kind_type}:")
            print(f"  - JSON files: {classification_stats[kind_type]['json']}")
            print(f"  - Image files: {classification_stats[kind_type]['image']}")
            print(f"  - Output directory: {os.path.join(OUTPUT_BASE_PATH, kind_type)}")
            print()
        
        print(f"Unclassified: {unclassified_count}")
        print(f"Errors: {error_count}")
        
        # 각 kind_type별 폴더 구조 확인
        print("\n=== Directory Structure ===")
        for kind_type in KIND_TYPES:
            kind_path = os.path.join(OUTPUT_BASE_PATH, kind_type)
            if os.path.exists(kind_path):
                print(f"{kind_type}/")
                print(f"  ├── json/ ({len(os.listdir(os.path.join(kind_path, 'json')))} files)")
                print(f"  └── image/ ({len(os.listdir(os.path.join(kind_path, 'image')))} files)")
        
    except Exception as error:
        print(f'Error during classification: {error}')

# 실행
if __name__ == "__main__":
    classify_files()