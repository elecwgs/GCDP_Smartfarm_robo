# main.py - 로컬 YOLO 모델 사용 버전
import cv2
import numpy as np
import time
import os
from datetime import datetime
from robot_control import RobotController
from lettuce_detection_leaf import LettuceDetection
from stereo_vision import StereoVision
from train import predict_multimodal

class LettuceHarvestSystem:
    def __init__(self, model_path, planting_date):
        """
        상추 수확 시스템 초기화
        Args:
            model_path: 학습된 YOLO 모델 경로
            planting_date: 파종일 (YYYY-MM-DD 형식)
        """
        self.detector = LettuceDetection(model_path=model_path)
        self.robot_controller = RobotController()
        self.stereo_vision = StereoVision()
        self.planting_date = planting_date
        self.system_ready = False
        self.model_path = model_path
        
        # 로그 디렉토리 생성
        self.log_dir = "logs"
        if not os.path.exists(self.log_dir):
            os.makedirs(self.log_dir)

    def initialize_system(self):
        """시스템 초기화"""
        print("\n=== 시스템 초기화 ===")
        
        # 모델 파일 존재 확인
        if not os.path.exists(self.model_path):
            print(f"✗ 모델 파일을 찾을 수 없습니다: {self.model_path}")
            return False
        
        try:
            # 로봇 컨트롤러 초기화
            if self.robot_controller.initialize_robot():
                self.robot_controller.calibrate_workspace()
                self.system_ready = True
                print("✓ 로봇 시스템 초기화 완료")
            else:
                print("✗ 로봇 초기화 실패")
                return False
                
            # 스테레오 비전 초기화
            if self.stereo_vision.initialize():
                print("✓ 스테레오 비전 초기화 완료")
            else:
                print("✗ 스테레오 비전 초기화 실패")
                
            print("✓ 시스템 준비 완료\n")
            return True
            
        except Exception as e:
            print(f"✗ 시스템 초기화 중 오류 발생: {e}")
            return False

    def capture_stereo_images(self):
        """스테레오 이미지 캡처"""
        try:
            # 실제 카메라에서 이미지 캡처하는 경우
            # left_img, right_img = self.stereo_vision.capture_stereo_pair()
            
            # 테스트용 이미지 파일 로드
            left_img = cv2.imread("left_camera.jpg")
            right_img = cv2.imread("right_camera.jpg")
            
            if left_img is not None and right_img is not None:
                print("✓ 스테레오 이미지 캡처 완료")
                return left_img, right_img
            else:
                print("✗ 스테레오 이미지 로드 실패")
                return None, None
                
        except Exception as e:
            print(f"✗ 이미지 캡처 중 오류 발생: {e}")
            return None, None

    def predict_growth_stage(self, image_path):
        """생육 단계 예측"""
        try:
            # multimodal 모델이 존재하는 경우
            if os.path.exists("best_multimodal_lettuce_model.pth"):
                result = predict_multimodal(
                    model_path="best_multimodal_lettuce_model.pth",
                    image_path=image_path,
                    planting_date=self.planting_date
                )
                return result["AI_예측_단계"]
            else:
                print("⚠️ 멀티모달 모델을 찾을 수 없습니다. 기본 수확기로 설정")
                return "수확기"
                
        except Exception as e:
            print(f"⚠️ 생육 단계 예측 중 오류: {e}. 기본 수확기로 설정")
            return "수확기"

    def save_harvest_log(self, leaves, harvest_count):
        """수확 로그 저장"""
        try:
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            log_file = os.path.join(self.log_dir, f"harvest_log_{timestamp}.txt")
            
            with open(log_file, 'w', encoding='utf-8') as f:
                f.write(f"수확 로그 - {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
                f.write(f"파종일: {self.planting_date}\n")
                f.write(f"검출된 잎 수: {len(leaves)}\n")
                f.write(f"수확 성공: {harvest_count}\n")
                f.write("=" * 50 + "\n")
                
                for i, leaf in enumerate(leaves):
                    f.write(f"{i+1}. 우선순위: {leaf['harvest_priority']}\n")
                    f.write(f"   위치: {leaf['center']}\n")
                    f.write(f"   크기: {leaf['area']:.0f}\n")
                    f.write(f"   신뢰도: {leaf['confidence']:.3f}\n")
                    f.write(f"   성숙도: {leaf.get('maturity_score', 0):.3f}\n")
                    f.write("-" * 30 + "\n")
                    
            print(f"✓ 수확 로그 저장: {log_file}")
            
        except Exception as e:
            print(f"⚠️ 로그 저장 중 오류: {e}")

    def run_single_cycle(self):
        """단일 수확 사이클 실행"""
        if not self.system_ready:
            print("시스템이 초기화되지 않았습니다.")
            return False

        print("\n=== 수확 사이클 시작 ===")
        
        # 스테레오 이미지 캡처
        left_img, right_img = self.capture_stereo_images()
        if left_img is None:
            return False

        # 임시 이미지 저장 (생육 단계 예측용)
        temp_image_path = "temp_left_image.jpg"
        cv2.imwrite(temp_image_path, left_img)

        # 생육 단계 예측
        stage = self.predict_growth_stage(temp_image_path)
        print(f"예측된 생육 단계: {stage}")

        # 수확기가 아닌 경우 건너뛰기
        if "수확기" not in stage:
            print("📌 아직 수확 단계가 아니므로 로봇팔 동작 생략")
            # 임시 파일 삭제
            if os.path.exists(temp_image_path):
                os.remove(temp_image_path)
            return False

        # 잎 검출 및 수확 시퀀스 생성
        leaves = self.detector.get_harvest_sequence(left_img)
        if not leaves:
            print("✗ 수확 가능한 잎이 없습니다")
            # 임시 파일 삭제
            if os.path.exists(temp_image_path):
                os.remove(temp_image_path)
            return False

        print(f"✓ 수확 대상 잎 수: {len(leaves)}")
        
        # 로봇 수확 실행
        try:
            harvest_count = self.robot_controller.execute_harvest_sequence(leaves, left_img, right_img)
            print(f"✅ 수확 완료: {harvest_count}/{len(leaves)}")
            
            # 수확 로그 저장
            self.save_harvest_log(leaves, harvest_count)
            
            # 임시 파일 삭제
            if os.path.exists(temp_image_path):
                os.remove(temp_image_path)
            
            return harvest_count > 0
            
        except Exception as e:
            print(f"✗ 수확 중 오류 발생: {e}")
            # 임시 파일 삭제
            if os.path.exists(temp_image_path):
                os.remove(temp_image_path)
            return False

    def test_leaf_detection(self, image_path):
        """잎 검출 테스트"""
        if not os.path.exists(image_path):
            print(f"✗ 이미지 파일을 찾을 수 없습니다: {image_path}")
            return
            
        try:
            image = cv2.imread(image_path)
            if image is None:
                print("✗ 이미지 로드 실패")
                return
                
            print("🔍 잎 검출 중...")
            leaves = self.detector.get_harvest_sequence(image)
            
            if not leaves:
                print("✗ 검출된 잎이 없습니다")
                return
                
            print(f"✓ 검출된 잎 수: {len(leaves)}")
            
            # 검출 결과 시각화
            vis_image = self.detector.visualize_detection_results(image, leaves)
            
            # 결과 이미지 저장
            output_path = f"detection_result_{datetime.now().strftime('%Y%m%d_%H%M%S')}.jpg"
            cv2.imwrite(output_path, vis_image)
            print(f"✓ 결과 이미지 저장: {output_path}")
            
            # 화면에 표시
            cv2.imshow("잎 검출 결과", vis_image)
            print("아무 키나 누르면 창이 닫힙니다...")
            cv2.waitKey(0)
            cv2.destroyAllWindows()
            
            # 검출 결과 상세 정보 출력
            print("\n=== 검출 결과 상세 정보 ===")
            for i, leaf in enumerate(leaves):
                print(f"{i+1}. 우선순위: {leaf['harvest_priority']}")
                print(f"   위치: ({leaf['center'][0]:.1f}, {leaf['center'][1]:.1f})")
                print(f"   크기: {leaf['area']:.0f} 픽셀")
                print(f"   신뢰도: {leaf['confidence']:.3f}")
                print(f"   성숙도: {leaf.get('maturity_score', 0):.3f}")
                print(f"   클래스: {leaf['class']}")
                print("-" * 40)
                
        except Exception as e:
            print(f"✗ 검출 테스트 중 오류 발생: {e}")

    def run_continuous_monitoring(self, interval=300):
        """연속 모니터링 모드 (interval: 초 단위)"""
        print(f"\n=== 연속 모니터링 시작 (간격: {interval}초) ===")
        print("Ctrl+C로 중단할 수 있습니다.")
        
        try:
            while True:
                success = self.run_single_cycle()
                if success:
                    print("✅ 수확 사이클 완료")
                else:
                    print("⚠️ 수확 사이클 건너뛰기")
                
                print(f"💤 {interval}초 대기 중...")
                time.sleep(interval)
                
        except KeyboardInterrupt:
            print("\n⏹️ 연속 모니터링 중단")
        except Exception as e:
            print(f"✗ 연속 모니터링 중 오류: {e}")

def main():
    """메인 함수"""
    # 설정값
    planting_date = "2025-06-20"  # 실제 파종일로 수정
    model_path = "runs/detect/train/weights/best.pt"  # 실제 모델 경로로 수정
    
    # 시스템 초기화
    harvest_system = LettuceHarvestSystem(
        model_path=model_path,
        planting_date=planting_date
    )

    print("\n=== 상추 스마트팜 수확 시스템 ===")
    print(f"모델 경로: {model_path}")
    print(f"파종일: {planting_date}")

    while True:
        print("\n[ 메뉴 ]")
        print("1. 시스템 초기화")
        print("2. 수확 사이클 실행")
        print("3. 잎 검출 테스트")
        print("4. 연속 모니터링")
        print("5. 시스템 상