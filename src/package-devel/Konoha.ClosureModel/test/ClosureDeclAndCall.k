import("Konoha.ClosureModel");

void test() {
	int x = 1;
	Func[int, int] f = function (int a) int {
		return a + x; /* a + 1 */
	}
	assert(f(10) == 11);
}

test();
