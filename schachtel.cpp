#include <stdio.h>

struct gst
{
	void ausg();
	~gst() {
		printf("zerstöre gst\n");
	}
};

void gst::ausg()
{
	printf("Ausgabe Grund\n");
}

struct f1cl:gst
{
	void ausg();
};

struct f2cl:gst
{
	void ausg();
  ~f2cl() {
		printf("zerstöre f2cl\n");
	}
};

void f1cl::ausg()
{
	printf("Ausgabe f1\n");
}

void f2cl::ausg()
{
	printf("Ausgabe f2\n");
}



main()
{
	/*
	gst gr;
	f1cl f1;
	*/
	f2cl f2;
	/*
	gr.ausg();
	f1.ausg();
	*/
	f2.ausg();
}
