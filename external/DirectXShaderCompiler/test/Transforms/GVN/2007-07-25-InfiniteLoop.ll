; RUN: opt < %s -basicaa -gvn -S | not grep "tmp10 ="

	%struct.INT2 = type { i32, i32 }
@blkshifts = external global %struct.INT2*		; <%struct.INT2**> [#uses=2]

define i32 @xcompact() {
entry:
	store %struct.INT2* null, %struct.INT2** @blkshifts, align 4
	br label %bb

bb:		; preds = %bb, %entry
	%tmp10 = load %struct.INT2*, %struct.INT2** @blkshifts, align 4		; <%struct.INT2*> [#uses=0]
	br label %bb
}
