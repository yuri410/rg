// RUN: not %clang_cc1 -fsyntax-only -fmessage-length 60 %s 2>&1 | FileCheck %s
// RUN: not %clang_cc1 -fsyntax-only -fmessage-length 0 %s 2>&1 | FileCheck %s

struct B { void f(); };
struct D1 : B {};
struct D2 : B {};
struct DD : D1, D2 {
  void g() { f(); }
  // Ensure that after line-wrapping takes place, we preserve artificial
  // newlines introduced to manually format a section of the diagnostic text.
  // CHECK: {{.*}}: error:
  // CHECK: struct DD -> struct D1 -> struct B
  // CHECK: struct DD -> struct D2 -> struct B
};

// A line longer than 4096 characters should cause us to suppress snippets no
// matter what -fmessage-length is set to.
#pragma clang diagnostic push
#pragma clang diagnostic warning "-Wconversion"
// CHECK: implicit conversion loses floating-point precision
// CHECK-NOT: static const float numbers[]
static const float numbers[] = {0.1764705882352941,0.1176470588235294,0.1176470588235294,0.1176470588235294,0.1176470588235294,0.3529411764705883,0.2352941176470588,0.1176470588235294,0.1176470588235294,0.1176470588235294,0.1176470588235294,0.4117647058823529,0.1176470588235294,0.1176470588235294,0.1176470588235294,0.1176470588235294,0.1764705882352941,0.3529411764705883,0.1764705882352941,0.1176470588235294,0.1176470588235294,0.1176470588235294,0.1764705882352941,0.4117647058823529,0.2352941176470588,0.1176470588235294,0.1176470588235294,0.1176470588235294,0.1764705882352941,0.4705882352941176,0.1176470588235294,0.1176470588235294,0.1176470588235294,0.1176470588235294,0.2352941176470588,0.4117647058823529,0.1764705882352941,0.1176470588235294,0.1176470588235294,0.1176470588235294,0.2352941176470588,0.4705882352941176,0.2352941176470588,0.1176470588235294,0.1176470588235294,0.1176470588235294,0.2352941176470588,0.5294117647058824,0.1176470588235294,0.1176470588235294,0.1176470588235294,0.1176470588235294,0.2941176470588235,0.4705882352941176,0.1764705882352941,0.1176470588235294,0.1176470588235294,0.1176470588235294,0.2941176470588235,0.5294117647058824,0.1176470588235294,0.1176470588235294,0.1176470588235294,0.1176470588235294,0.3529411764705883,0.5294117647058824,0.4117647058823529,0.1176470588235294,0.1176470588235294,0.1764705882352941,0.1764705882352941,0.1176470588235294,0.1176470588235294,0.1176470588235294,0.1176470588235294,0.1764705882352941,0.1764705882352941,0.3529411764705883,0.1764705882352941,0.1176470588235294,0.1176470588235294,0.1764705882352941,0.1764705882352941,0.4117647058823529,0.4117647058823529,0.1176470588235294,0.1176470588235294,0.1764705882352941,0.2352941176470588,0.1764705882352941,0.1176470588235294,0.1176470588235294,0.1176470588235294,0.1764705882352941,0.2352941176470588,0.4117647058823529,0.1764705882352941,0.1176470588235294,0.1176470588235294,0.1764705882352941,0.2352941176470588,0.4705882352941176,0.4117647058823529,0.1176470588235294,0.1176470588235294,0.1764705882352941,0.2941176470588235,0.2352941176470588,0.1176470588235294,0.1176470588235294,0.1176470588235294,0.1764705882352941,0.2941176470588235,0.4705882352941176,0.1764705882352941,0.1176470588235294,0.1176470588235294,0.1764705882352941,0.2941176470588235,0.5294117647058824,0.4117647058823529,0.1176470588235294,0.1176470588235294,0.1764705882352941,0.3529411764705883,0.2941176470588235,0.1176470588235294,0.1176470588235294,0.1176470588235294,0.1764705882352941,0.3529411764705883,0.5294117647058824,0.3529411764705883,0.1176470588235294,0.1176470588235294,0.2352941176470588,0.2352941176470588,0.1176470588235294,0.4117647058823529,0.1176470588235294,0.1176470588235294,0.2352941176470588,0.2352941176470588,0.1764705882352941,0.1176470588235294,0.1176470588235294,0.1176470588235294,0.2352941176470588,0.2352941176470588,0.4117647058823529,0.3529411764705883,0.1176470588235294,0.1176470588235294,0.2352941176470588,0.2941176470588235,0.1764705882352941,0.4117647058823529,0.1176470588235294,0.1176470588235294,0.2352941176470588,0.2941176470588235,0.2352941176470588,0.1176470588235294,0.1176470588235294,0.1176470588235294,0.2352941176470588,0.2941176470588235,0.4705882352941176,0.3529411764705883,0.1176470588235294,0.1176470588235294,0.2352941176470588,0.3529411764705883,0.2352941176470588,0.1176470588235294,0.1176470588235294,0.1176470588235294,0.2352941176470588,0.3529411764705883,0.5294117647058824,0.3529411764705883,0.1176470588235294,0.1176470588235294,0.2352941176470588,0.4117647058823529,0.2941176470588235,0.2941176470588235,0.1176470588235294,0.1176470588235294,0.2941176470588235,0.2941176470588235,0.1176470588235294,0.3529411764705883,0.1176470588235294,0.1176470588235294,0.2941176470588235,0.2941176470588235,0.1764705882352941,0.2941176470588235,0.1176470588235294,0.1176470588235294,0.2941176470588235,0.3529411764705883,0.1764705882352941,0.3529411764705883,0.1176470588235294,0.1176470588235294,0.2941176470588235,0.3529411764705883,0.2352941176470588,0.2941176470588235,0.1176470588235294,0.1176470588235294,0.2941176470588235,0.4117647058823529,0.2352941176470588,0.2941176470588235,0.1176470588235294,0.1176470588235294,0.2941176470588235,0.4705882352941176,0.2941176470588235,0.2352941176470588,0.1176470588235294,0.1176470588235294,0.3529411764705883,0.3529411764705883,0.1176470588235294,0.2941176470588235,0.1176470588235294,0.1176470588235294,0.3529411764705883,0.3529411764705883,0.1764705882352941,0.2352941176470588,0.1176470588235294,0.1176470588235294,0.3529411764705883,0.4117647058823529,0.1764705882352941,0.2941176470588235,0.1176470588235294,0.1176470588235294,0.3529411764705883,0.4117647058823529,0.2352941176470588,0.2352941176470588,0.1176470588235294,0.1176470588235294,0.3529411764705883,0.4705882352941176,0.2352941176470588,0.2352941176470588,0.1176470588235294,0.1176470588235294,0.3529411764705883,0.5294117647058824,0.2941176470588235,0.1764705882352941,0.1176470588235294,0.1176470588235294,0.4117647058823529};
#pragma clang diagnostic pop
