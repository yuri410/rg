@zed = global i32 42
@foo = alias i32* @zed
@foo2 = alias bitcast (i32* @zed to i16*)
