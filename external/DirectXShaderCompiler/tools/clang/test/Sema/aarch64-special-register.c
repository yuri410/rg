// RUN: %clang_cc1 -ffreestanding -fsyntax-only -verify -triple aarch64 %s

void string_literal(unsigned v) {
  __builtin_arm_wsr(0, v); // expected-error {{expression is not a string literal}}
}

void wsr_1(unsigned v) {
  __builtin_arm_wsr("sysreg", v);
}

void wsrp_1(void *v) {
  __builtin_arm_wsrp("sysreg", v);
}

void wsr64_1(unsigned long v) {
  __builtin_arm_wsr64("sysreg", v); //expected-error {{invalid special register for builtin}}
}

unsigned rsr_1() {
  return __builtin_arm_rsr("sysreg");
}

void *rsrp_1() {
  return __builtin_arm_rsrp("sysreg");
}

unsigned long rsr64_1() {
  return __builtin_arm_rsr64("sysreg"); //expected-error {{invalid special register for builtin}}
}

void wsr_2(unsigned v) {
  __builtin_arm_wsr("0:1:2:3:4", v);
}

void wsrp_2(void *v) {
  __builtin_arm_wsrp("0:1:2:3:4", v);
}

void wsr64_2(unsigned long v) {
  __builtin_arm_wsr64("0:1:2:3:4", v);
}

unsigned rsr_2() {
  return __builtin_arm_rsr("0:1:2:3:4");
}

void *rsrp_2() {
  return __builtin_arm_rsrp("0:1:2:3:4");
}

unsigned long rsr64_2() {
  return __builtin_arm_rsr64("0:1:2:3:4");
}

void wsr_3(unsigned v) {
  __builtin_arm_wsr("0:1:2", v); //expected-error {{invalid special register for builtin}}
}

void wsrp_3(void *v) {
  __builtin_arm_wsrp("0:1:2", v); //expected-error {{invalid special register for builtin}}
}

void wsr64_3(unsigned long v) {
  __builtin_arm_wsr64("0:1:2", v); //expected-error {{invalid special register for builtin}}
}

unsigned rsr_3() {
  return __builtin_arm_rsr("0:1:2"); //expected-error {{invalid special register for builtin}}
}

void *rsrp_3() {
  return __builtin_arm_rsrp("0:1:2"); //expected-error {{invalid special register for builtin}}
}

unsigned long rsr64_3() {
  return __builtin_arm_rsr64("0:1:2"); //expected-error {{invalid special register for builtin}}
}
