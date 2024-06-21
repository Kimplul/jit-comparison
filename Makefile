all:
	$(MAKE) -C loop
	$(MAKE) -C fib

.DEFAULT:
	$(MAKE) -C loop $<
	$(MAKE) -C fib $<
