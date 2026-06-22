# SEWHA Source Code Standard (EIS-R2501)

[cite_start]이 가이드는 프로젝트 품질의 일관성과 유지보수 효율을 위해 준수해야 할 표준입니다[cite: 38].

## 1. Software Coding Style
### 1.1 이름 규칙 (Naming)
- [cite_start]**함수명**: 단어의 시작은 대문자로 하되, API와 구분을 위해 앞에 소문자 'v'를 붙임 (예: `vSetClock`)[cite: 45, 46].
- [cite_start]**헤더명**: 표준 헤더는 `< >`, 자체 생성 헤더는 `" "`를 사용함[cite: 50].
- [cite_start]**변수명**: Camel Case를 사용하며, 전역 변수는 앞에 'g'를 붙임 (예: `gLoopCount`)[cite: 55, 56].
- [cite_start]**상수명**: 대문자와 언더바(`_`)를 사용하여 표기함 (예: `D_NETWORK_BUFFER_SIZE`)[cite: 64, 68].

### 1.2 서식 (Formatting)
- [cite_start]**들여쓰기**: Tab을 사용하여 depth별로 구분함[cite: 70].
- [cite_start]**띄어쓰기**: 함수명, 배열, 조건문 다음에 띄어쓰기 없이 바로 괄호`(`를 시작함 (예: `for(...)`, `vComCheck(0)`)[cite: 72, 74].
- [cite_start]**중괄호**: GNU Style을 적용하여 중괄호를 다음 라인으로 내려서 시작함[cite: 76].
- [cite_start]**헤더 가드**: 모든 헤더 파일에는 중복 포함 방지를 위해 `#ifndef`, `#define` 가드를 사용함[cite: 87, 89].
- [cite_start]**인코딩**: 모든 파일은 UTF-8 인코딩을 사용함[cite: 93].

## 2. Software Coding Rule (MISRA-C:2012)
[cite_start]총 143개 규칙 중 핵심 87개 규칙을 적용함[cite: 99]. 아래는 에이전트가 즉시 준수해야 할 핵심 항목임:

- [cite_start]**데이터 타입**: 모든 변수는 읽기 전 반드시 할당(초기화)되어야 함 (Mandatory)[cite: 103, 109].
- [cite_start]**상수 접미사**: Unsigned integer 상수에는 반드시 'u' 또는 'U'를 붙여야 함 (Required)[cite: 102, 111].
- [cite_start]**메모리 관리**: `stdlib.h`의 메모리 할당(`malloc`) 및 해제(`free`) 함수 사용을 금지함 (Required)[cite: 108, 111].
- **제어 흐름**: 
  - [cite_start]재귀 호출(Recursion)은 직/간접 모두 금지함 (Required)[cite: 106].
  - [cite_start]`goto` 문 사용은 원칙적으로 권장하지 않음 (Advisory)[cite: 105, 112].
  - [cite_start]모든 `switch` 절은 반드시 `break`로 끝나야 함 (Required)[cite: 106].
- **연산**: 
  - [cite_start]`sizeof` 연산자를 배열로 선언된 함수 파라미터에 사용 금지 (Mandatory)[cite: 105].
  - [cite_start]함수 포인터와 다른 타입 간의 형 변환 금지 (Required)[cite: 104].