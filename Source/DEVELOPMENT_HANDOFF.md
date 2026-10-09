# GameAI3 개발 진행 상황과 작업 인계

작성 기준일: 2026-10-09 (한국 시간)

이 문서는 노트북의 Codex가 기존 대화 없이 프로젝트를 이해하고 후속 작업을 진행하기 위한 인계 자료다. 현재 구현 상태와 사용자의 작업 조건을 기록한다. 아래 개선 제안은 아직 구현하지 않았으며, 문서를 읽었다는 이유만으로 수정 작업을 시작하지 않는다. 후속 사용자의 요청과 실제 최신 소스코드를 우선한다.

## 1. 프로젝트 목적

상인·도둑·경비병이 등장하는 동일한 게임 시나리오를 두 가지 게임 AI 구조로 구현하고 비교한다.

- `FSMGame`: FSM With Messaging.
- `BTGame`: Behavior Tree와 공유 Blackboard.

사용자는 코드를 직접 분석하고 분석보고서를 작성할 예정이다. 행동을 임의로 바꾸지 않고, 구조의 차이를 읽기 쉽게 표현하는 것이 중요하다.

보고서 원본은 저장소 루트의 `분석보고서.docx`다. 문서 제목은 「동일 시나리오의 FSM With Messaging 및 Behavior Tree 구현·비교」다. 보고서의 시나리오와 등장인물 설명을 읽고 FSM 구현에 반영했으며, 보고서 자체는 수정하지 않았다.

## 2. 경로와 개발 환경

이 Markdown 파일은 저장소의 `Source` 폴더에 있다. 아래 경로는 이 파일이 있는 폴더를 기준으로 한다. 노트북에서는 실제 체크아웃 위치에 맞춰 해석한다.

```text
GameAI3/
├─ 분석보고서.docx
└─ Source/
   ├─ DEVELOPMENT_HANDOFF.md
   ├─ FSMGame/
   │  ├─ FSMGame.slnx
   │  └─ FSMGame/
   │     ├─ FSMGame.vcxproj
   │     ├─ Entity.h / Entity.cpp
   │     ├─ State.h / State.cpp
   │     ├─ StateMachine.h / StateMachine.cpp
   │     ├─ Telegram.h / Telegram.cpp
   │     ├─ MessageManager.h / MessageManager.cpp
   │     ├─ GameConfig.h
   │     ├─ main.cpp
   │     └─ pch.h / pch.cpp
   └─ BTGame/
      └─ BTGame/
         ├─ BTGame.slnx
         └─ BTGame/
            ├─ BTGame.vcxproj
            ├─ BehaviorTree.h / BehaviorTree.cpp
            ├─ Blackboard.h / Blackboard.cpp
            ├─ Entity.h / Entity.cpp
            ├─ GameConfig.h
            ├─ main.cpp
            └─ pch.h / pch.cpp
```

BTGame의 중복된 폴더 계층은 실제 배치다.

- Windows용 Visual Studio C++ 콘솔 프로젝트.
- 두 프로젝트 모두 C++20, MSVC 툴셋 `v145`, Debug/Release 및 Win32/x64 구성.
- 현재 한글 소스는 CP949 기준이다. UTF-8로 잘못 읽으면 한글이 깨져 보일 수 있으므로 수정 전에 인코딩을 확인한다.
- 두 `main.cpp`는 `SetConsoleOutputCP(949)`를 사용한다.
- 현재 FSM 프로젝트에는 `/source-charset:.949 /execution-charset:.949` 설정이 명시되어 있다. BT 프로젝트에는 같은 옵션이 명시되어 있지 않다. 실제 프로젝트 설정을 기준으로 작업한다.
- 이 인계 문서는 UTF-8로 저장한다.

## 3. 사용자가 명시한 작업 조건

1. **빌드와 실행은 사용자에게 맡긴다.** 사용자가 토큰 소모를 이유로 직접 요청했다. 별도 지시가 없으면 빌드하거나 게임을 실행하지 않는다.
2. 행동, 확률, 시간 조건, 종료 조건을 임의로 변경하지 않는다.
3. 파일을 불필요하게 늘리지 않는다. 기존 파일의 개명·통합과 불필요한 파일 삭제를 활용한다.
4. 사용자가 직접 분석할 수 있도록 클래스와 함수 역할을 명확히 하고 가독성을 유지한다.
5. 두 솔루션은 별도 구현이다. 한쪽 작업을 이유로 다른 솔루션까지 수정하지 않는다.
6. 이번 인계 문서에 남기는 개선 제안은 **상인의 두 상태에 중복된 메시지 처리** 한 가지뿐이다.

## 4. 공통 시나리오와 수치

### 행동 흐름

- 상인은 물건을 판매하며 기다린다. 현재 구현에서는 일반 손님 구매로 재고가 줄어드는 별도 로직은 없으며, 절도 한 번에 물건 한 개가 상인에게서 도둑으로 이동한다.
- 도둑은 일정 Tick 동안 배회한 뒤 물건을 하나 훔치고 다시 배회한다.
- 상인은 도난을 일정 확률로 발견하고 경비병에게 도움을 요청한다.
- 경비병이 출동하면 도둑은 도주한다. 도둑은 경비병의 복귀가 완료될 때까지 도주를 유지한다.
- 경비병은 추격 중 매 Tick 일정 확률로 도둑을 잡는다. 추격 제한 시간이 끝나면 복귀한다.
- 경비병이 복귀를 마치면 상인은 판매, 도둑은 배회, 경비병은 순찰을 재개한다.

### 현재 GameConfig

두 솔루션의 `GameConfig.h` 값은 다음과 같다.

| 설정 | 값 | 의미 |
|---|---:|---|
| `TickIntervalMs` | 100 | Tick 간 대기 시간의 기준값(ms) |
| `InitialItems` | 10 | 상인의 초기 물건 수 |
| `WanderTicks` | 3 | 절도에 앞서 배회하는 Tick 수 |
| `ChaseTicks` | 4 | 추격 최대 Tick 수 |
| `ReturnTicks` | 2 | 경비병의 복귀에 필요한 Tick 수 |
| `NoticeTheftPercent` | 50 | 도난 발생 시 상인의 발견 확률(%) |
| `CatchThiefPercent` | 20 | 추격 중 매 Tick 체포 확률(%) |

초기 구현 과정에서는 Tick 간격을 1000ms로 두었으나, **현재 소스의 값은 100ms**다. 오래된 대화의 수치로 되돌리지 않는다.

확률은 `std::rand() % 100 < 확률값`으로 판정하고, `main()`에서 시간 기반으로 `std::srand()`를 한 번 호출한다.

### 종료와 결과

다음 조건 중 **하나라도** 만족하면 종료한다.

1. 상인에게 남은 물건이 0개.
2. 경비병이 도둑을 체포함.

결과에는 종료 원인, 상인에게 남은 물건 수, 도둑이 훔친 물건 수를 출력한다. 체포 시 훔친 물건을 상인에게 반환하는 규칙은 구현되어 있지 않다.

### Tick과 출력 규칙

- 인물 업데이트 순서는 상인 → 도둑 → 경비병이다.
- 각 인물은 한 Tick에 행동을 한 번 실행하고 콘솔에 한 줄 출력한다.
- 중간에 종료 조건이 충족되면 그 Tick의 뒤쪽 인물은 실행하지 않는다. 따라서 마지막 Tick에는 세 인물이 모두 출력하지 않을 수 있다.
- 배회 3회가 끝난 같은 Tick에 바로 절도 대사를 추가 출력하지 않는다. 절도는 다음 Tick에 실행한다.
- 추격 종료 Tick에는 추격 종료 대사를 출력하고, 복귀 행동은 다음 Tick부터 실행한다.
- 현재 FSM의 반복문은 `Sleep(GameConfig::TickIntervalMs)`를, BT는 `std::chrono::steady_clock`과 `sleep_until()`을 사용한다. 판정 시간은 양쪽 모두 Tick 횟수에 기반한다.

## 5. FSMGame 구현 상태

### 인물과 상태

`CBaseEntity`를 기반으로 `CMerchant`, `CThief`, `CGuard`가 있으며, 각 인물은 `CStateMachine<자기 타입>`을 소유한다.

| 인물 | 일반 상태 |
|---|---|
| 상인 | `SELLING`, `CALLING_GUARD`, `WAITING` |
| 도둑 | `WANDERING`, `STEALING`, `ESCAPING` |
| 경비병 | `PATROLLING`, `CHASING`, `RETURNING` |

- `State.h`에 상태 인터페이스와 구체 상태 선언, `State.cpp`에 행동·전환·메시지 처리 구현이 있다.
- 상태 객체는 정적 인스턴스로 공유한다. 인물별 데이터와 경과 Tick은 Entity 쪽에 저장한다.
- `CStateMachine`은 현재·이전·전역 상태를 보관한다.
- 전환은 기존 상태 `Exit()` → 새 상태 `Enter()` 순서다.
- `Update()`는 전역 상태 `Execute()`를 먼저, 현재 상태 `Execute()`를 다음에 호출한다.
- 메시지는 현재 상태가 먼저 처리하고, 처리하지 않으면 전역 상태에 전달한다.
- 템플릿 구현은 `StateMachine.h`에 있다. `StateMachine.cpp`에는 설명과 include만 있다.

### GLOBAL_STATE의 실제 사용

**현재 전역 상태를 등록하고 사용하는 인물은 도둑뿐이다.**

- `CThief` 생성자에서 `THIEF_GLOBAL_STATE::Instance()`를 등록한다.
- `THIEF_GLOBAL_STATE::Enter()`, `Execute()`, `Exit()`는 비어 있다.
- `OnMessage()`는 실제로 사용된다.
  - 경비병의 `MSG_GUARD_CHASING` 수신: `ESCAPING`으로 전환.
  - 경비병의 `MSG_GUARD_RETURNING` 수신: 도주 중이라면 `WANDERING`으로 전환.
- 상인과 경비병은 현재 전역 상태를 등록하지 않는다.

### 메시지와 데이터 흐름

`MessageManager`는 ID별 인물 등록·조회·제거와 Telegram 대기열을 관리한다. 즉시 메시지도 큐에 넣으며, 지연 시각을 지정한 메시지를 처리할 수 있는 구조다. 현재 시나리오에서는 지연 메시지를 사용하지 않는다.

| 메시지 | 흐름과 결과 |
|---|---|
| `MSG_ITEM_STOLEN` | 도둑 → 상인. 상인이 판매 중이고 재고가 남아 있으면 발견 확률 판정 |
| `MSG_THIEF_SPOTTED` | 상인 → 경비병. 순찰 중인 경비병이 추격 진입 |
| `MSG_GUARD_CHASING` | 경비병 → 도둑. 도둑이 도주 진입 |
| `MSG_GUARD_RETURNING` | 경비병 → 도둑과 상인. 복귀 완료 후 배회·판매 재개 |

- 절도 시 `CThief::StealOneItem()`이 참조 중인 상인의 물건 한 개를 가져오고 자신의 획득량을 늘린 뒤 메시지를 보낸다.
- 체포 시 경비병은 참조 중인 도둑의 체포 여부를 변경한다.
- `CALLING_GUARD::Enter()`에서 경비병 호출 메시지를 보낸다. 호출 대사는 다음 `Execute()`에서 출력한다.
- `CHASING::Enter()`에서 도둑에게 추격 메시지를 보낸다.
- 경비병의 복귀 메시지는 `RETURNING` 진입 시가 아니라 **복귀가 완료되어 `PATROLLING`으로 돌아가는 시점**에 보낸다.
- `main.cpp`의 `Tick()`은 시작 시 메시지 시간을 갱신하고, 인물 업데이트 후 대기 메시지를 처리한다.
- 메시지 처리 중 새로 발생한 즉시 메시지도 같은 처리 루프에서 전달된다. 이 과정에서 상태 진입은 가능하지만, 새 상태의 행동 `Execute()`는 다음 인물 업데이트까지 실행하지 않는다.

## 6. BTGame 구현 상태

사용자가 FSM과 같은 소스를 복사하여 만든 별도 솔루션을 Behavior Tree로 변환했다. 상태 번호를 바꾸는 방식 대신 실제 조건·복합·행동 노드를 구성했다.

### 파일 재구성

| 기존 파일 | 변경 결과 |
|---|---|
| `StateMachine.h / .cpp` | `BehaviorTree.h / .cpp`로 개명하고 BT 노드 구현 |
| `MessageManager.h / .cpp` | `Blackboard.h / .cpp`로 개명하고 공유 정보 구현 |
| `State.h / .cpp` | 삭제. 인물 행동을 `Entity.cpp`로 통합 |
| `Telegram.h / .cpp` | 삭제. 메시지 시스템 제거 |
| `Entity.h / .cpp` | 인물별 트리 생성과 행동 함수로 재구성 |
| `main.cpp` | Blackboard와 인물 생성, Tick 끝 정보 반영으로 변경 |

소스·헤더 수는 14개에서 10개로 줄었다. 작업 당시 비어 있던 `BTGame.vcxproj`의 소스 목록과 `.filters`에 현재 파일들을 등록했다.

### 공통 노드

`BehaviorTree.h / .cpp`에 다음이 구현되어 있다.

- `NodeStatus`: `Success`, `Failure`, `Running`.
- `CSelector`: 매 Tick 앞쪽 자식부터 평가한다. 선택되지 않은 가지는 Reset하여 이전 진행을 취소한다.
- `CSequence`: Running인 자식 위치를 기억하고 다음 Tick에 이어서 실행한다.
- `CReactiveSequence`: 매 Tick 처음부터 조건을 다시 평가한다. 조건이 실패하면 하위 행동을 Reset한다.
- `CCondition`: bool 조건을 Success/Failure로 변환한다.
- `CAction`: 인물의 행동 함수를 호출한다. 완료 또는 취소 시 필요한 타이머 초기화 콜백을 지원한다.
- `CBehaviorTree`: 인물별 루트 노드를 소유하고 Tick한다.

`TickContext::actionExecuted`로 **인물별 한 Tick에 실제 행동 하나**만 허용한다. Sequence에서 앞 행동이 성공해도 다음 행동은 Running으로 대기한 뒤 다음 Tick에 실행한다. 이 규칙으로 기존 FSM의 출력·행동 간격을 유지한다.

### 인물별 트리

트리 생성은 `Entity.cpp`의 각 `BuildBehaviorTree()`에 있으며, 인물 행동은 같은 파일의 별도 이름 있는 함수로 구현되어 있다.

```text
상인 Selector
├─ ReactiveSequence
│  ├─ 도움 요청이 유지 중인가?
│  └─ Sequence: CallGuard → WaitForGuard
└─ Sell

도둑 Selector
├─ ReactiveSequence
│  ├─ 경비병의 출동이 유지 중인가?
│  └─ Escape
└─ Sequence: Wander → Steal

경비병 Selector
├─ ReactiveSequence
│  ├─ 도움 요청이 있는가?
│  └─ Sequence: Chase → ReturnToPatrol
└─ Patrol
```

- 도둑의 최우선 도주 가지가 FSM의 전역 메시지 반응에 대응한다.
- 배회·절도 중 도주 가지가 선택되면 하위 가지 진행과 배회 타이머를 초기화한다. 복귀 이후 배회를 처음부터 시작한다.
- 경비병의 Chase가 Success를 반환하는 것은 **추격 단계가 완료됨**을 의미한다. 체포뿐 아니라 추격 시간 초과 후에도 다음 복귀 노드로 진행한다.
- 체포 시에는 게임 종료 판정 때문에 후속 복귀 행동을 실행하지 않는다.

### Blackboard

Blackboard는 재고, 도둑의 획득량, 체포 여부, 경비병 도움 요청을 공유한다. Entity ID나 Telegram을 사용하지 않는다.

- `StealOneItem()`: 재고를 한 개 이동하고 이번 Tick의 도난 발생을 기록한다.
- `CatchThief()`: 체포 결과를 즉시 반영한다.
- `CompleteGuardReturn()`: 복귀 완료를 이번 Tick의 반영 대기로 기록한다.
- `CommitTick()`: 인물 업데이트가 끝난 뒤 도난 발견 확률을 판정하고, 도움 요청 또는 복귀 완료를 공유한다.
- 도움 요청은 발견 시점부터 복귀 완료까지 유지된다. 경비병이 복귀 중일 때도 도둑이 계속 도주하게 하는 조건이다.
- 도난 발견 판정과 정보 반영을 Tick 끝에 두어 기존 메시지 처리 시점 및 난수 사용 순서를 유지하도록 작성했다.

## 7. 검증 범위

빌드와 게임 실행은 사용자의 요청에 따라 수행하지 않았다. **컴파일 성공 또는 런타임 동작 검증 완료로 표현하지 않는다.** 사용자가 실행한 결과가 있는지는 별도로 확인해야 한다.

수행한 정적 확인은 다음과 같다.

- FSM 상태 선언과 구현 연결 및 프로젝트 파일 등록 확인.
- BT 프로젝트·필터 XML, 소스 파일 등록, 로컬 include 대상 존재 여부 확인.
- BT 소스에 기존 FSM·Telegram·MessageManager 참조가 남아 있지 않음을 확인.
- BT 변환 전후 등장인물 대사와 결과 출력 문자열 비교. 프로그램 제목만 Behavior Tree로 변경.
- BT 변환 전후 `GameConfig.h` 내용 일치 확인.
- BT 변환 작업 전후 FSM 소스 파일 지문 일치 확인.
- Tick별 행동 진행과 메시지/Blackboard 반영 순서를 소스 수준에서 검토.

후속 확인에서 이전 답변 당시와 달라진 소스도 확인했다. 현재 FSM `State.cpp`는 함수 서식이 정리되어 있으며, FSM `main.cpp`의 대기는 `Sleep()` 방식이다. 실제 파일을 다시 읽고 작업하며 이전 답변의 줄 번호나 파일 내용을 그대로 가정하지 않는다.

## 8. 저장할 개선 제안 한 가지

### 상인의 동일한 메시지 처리가 두 상태에 중복되어 있음

현재 `FSMGame/FSMGame/State.cpp`의 다음 두 함수는 동일한 처리를 갖는다.

- `CALLING_GUARD::OnMessage()`
- `WAITING::OnMessage()`

두 함수 모두 경비병에게서 `MSG_GUARD_RETURNING`을 받으면 `SELLING`으로 전환하고 `true`를 반환한다.

제안은 상인의 전역 상태 `MERCHANT_GLOBAL_STATE`에 이 공통 반응을 모으는 것이다.

후속 구현 시 유지해야 할 범위:

1. 상인 생성자에서 전역 상태를 등록한다.
2. 전역 상태의 `OnMessage()`에서 발신자가 경비병이고 메시지가 `MSG_GUARD_RETURNING`인지 확인한다.
3. **현재 상태가 `CALLING_GUARD` 또는 `WAITING`일 때만** `SELLING`으로 전환한다.
4. 두 개별 상태의 중복 코드를 제거하고, 공통 메시지가 전역 상태까지 전달되도록 처리하지 않은 메시지에는 `false`를 반환한다.
5. `SELLING`의 도난 발견 확률 판정, 호출 메시지 발신 시점, Tick 순서, 출력 내용은 유지한다.
6. 기존 `State.h / State.cpp`에 선언·구현할 수 있으므로 새 파일은 필요하지 않다.

**상태: 제안만 설명했으며 아직 구현하지 않았다.** 이번 요청은 개발 현황의 문서 저장이며, 개선 구현을 요청한 것이 아니다.
