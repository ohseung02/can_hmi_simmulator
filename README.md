# CAN-based Vehicle Infotainment (HMI) Simulator

## 📌 Project Overview (프로젝트 개요)
이 프로젝트는 실제 차량의 내부 통신망인 **CAN(Controller Area Network) 버스** 데이터를 기반으로 동작하는 **차량 인포테인먼트(HMI) 시스템**을 모사한 풀스택 웹 시뮬레이터입니다. C++로 구축된 고성능 백엔드 서버가 CAN 신호를 수신·파싱하여 WebSocket을 통해 웹 프론트엔드로 실시간 전송하며, 실제 아두이노 기반의 하드웨어 연동(HIL)까지 지원합니다.

## 🛠 Tech Stack (기술 스택)
- **Backend (Server)**: C++17, CMake, Crow (WebSocket & HTTP Framework), Asio (Serial Communication)
- **Frontend (Web UI)**: Vanilla HTML5, CSS3, JavaScript (ES6+), WebSocket API
- **Hardware (Optional)**: Arduino UNO, MCP2515 CAN Shield, DHT11 (Temperature Sensor)
- **Version Control**: Git

## 🔑 Key Features (주요 구현 기능)

### 1. 실시간 차량 상태 시각화 (Real-time HMI)
- **제어계 (Controls)**: 스티어링 휠 조향각(-180~180도), 브레이크 압력, 스로틀(가속) 개방량을 직관적인 그래픽과 게이지바로 표현.
- **계기판 (Cluster)**: 차량 속도 및 RPM, 기어 상태(P/R/N/D), 실내 온도 실시간 렌더링.
- *디자인 원칙*: 최신 차량 트렌드에 맞춘 글래스모피즘(Glassmorphism) 기반 프리미엄 다크 모드 UI 적용.

### 2. 고성능 C++ WebSocket 서버
- 초당 수십~수백 건씩 쏟아지는 CAN 메시지를 지연 없이 프론트엔드로 브로드캐스트하기 위해 **Crow 프레임워크 기반의 비동기 WebSocket 서버**를 직접 구축.

### 3. 하드웨어 인 더 루프 (HIL) 시뮬레이션 지원
- **아두이노 CAN 쉴드 연동**: 실제 CAN 버스에서 캡처된 하드웨어 신호를 아두이노가 읽어 USB 시리얼 포트(COM)를 통해 C++ 서버로 전송합니다.
- `Asio` 라이브러리를 사용해 C++ 서버 내에 별도의 스레드를 두어 시리얼 데이터를 실시간 파싱(Parsing)하도록 구현했습니다.

### 4. 주행 로그 리플레이 (CAN Log Replay)
- 하드웨어가 없는 상황에서도 테스트가 가능하도록, 녹화된 **주행 로그(CSV 형식)**를 시간에 맞춰 자동 재생하는 리플레이 엔진(`LogReplayer`)을 구현했습니다.

## 📈 Development Process (개발 과정 및 문제 해결)
- **요구사항 분석 및 설계**: 단방향 UI 구성에서 벗어나, C++ 백엔드와 웹 간의 실시간 양방향 통신 구조를 기획.
- **Phase 1 (소프트웨어 시뮬레이션)**: C++ 백엔드 로직 작성 및 웹 HMI 구현. 수동으로 CAN ID를 입력해 UI가 즉각 반응하도록 구성.
- **Phase 2 (하드웨어 통합 및 고도화)**: 실제 아두이노 통신 코드(TX/RX) 분석을 토대로 시리얼 통신 모듈 및 주행 로그 리플레이 기능 추가.
- **트러블슈팅 (Deadlock 해결)**: WebSocket 커넥션과 CAN 메시지 브로드캐스트 과정에서 발생한 스레드 데드락(Deadlock) 이슈를 `std::recursive_mutex`를 도입하여 안전하게 해결하고 서버 안정성을 확보함.

## 🚀 How to Run (실행 방법)
1. **Build**: `cmake -B build` 및 `cmake --build build --config Release`
2. **Run Server**: `./build/CanServer.exe`
3. **Open UI**: 브라우저에서 `frontend/index.html` 실행
