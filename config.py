# config.py - 시스템 설정 파일

# Roboflow 설정
ROBOFLOW_CONFIG = {
    'api_key': 'your_actual_roboflow_api_key',  # 실제 API 키로 변경
    'workspace': 'your_workspace_name',         # 실제 워크스페이스 이름으로 변경
    'project': 'lettuce-leaf-detection',        # 실제 프로젝트 이름으로 변경
    'version': 1                                # 실제 버전 번호로 변경
}

# 로봇 팔 설정
ROBOT_CONFIG = {
    'serial_port': '/dev/ttyUSB0',
    'linear_actuator_port': '/dev/ttyUSB1',
    'home_position': [0.2, 0.0, 0.15],
    'max_speed': 0.1,
    'approach_height': 0.05,
    'retreat_height': 0.08
}

# 카메라 설정
CAMERA_CONFIG = {
    'left_camera_id': 0,
    'right_camera_id': 1,
    'image_width': 640,
    'image_height': 480,
    'calibration_file': 'stereo_calibration.json'
}

# 검출 설정
DETECTION_CONFIG = {
    'confidence_threshold': 0.5,
    'maturity_threshold': 0.7,
    'size_threshold': 1000,
    'max_harvest_leaves': 10
}

# 시스템 경로
PATHS = {
    'test_images': 'test_images/',
    'calibration_data': 'calibration/',
    'logs': 'logs/'
}