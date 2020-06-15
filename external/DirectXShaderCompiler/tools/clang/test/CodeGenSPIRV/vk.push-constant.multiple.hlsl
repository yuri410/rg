// Run: %dxc -T vs_6_0 -E main

struct S {
    float4 f;
};

[[vk::push_constant]]
S pcs1;

[[vk::push_constant]] // error
S pcs2;

[[vk::push_constant]] // error
S pcs3;

float main() : A {
    return 1.0;
}

// CHECK: :10:3: error: cannot have more than one push constant block
// CHECK: :7:3: note: push constant block previously defined here

// CHECK: :13:3: error: cannot have more than one push constant block
// CHECK: :7:3: note: push constant block previously defined here
