#include "systemc.h"
#include "visualize.h"

SC_MODULE(A)
{
    sc_out<int> out;

    SC_CTOR(A) : out("out")
    {

    }
};

SC_MODULE(B)
{
    sc_in<int> in;

    SC_CTOR(B) : in ("in")
    {

    }
};


SC_MODULE(D)
{
    sc_in<int> in;
    A alpha;
    B beta;

    sc_signal<int> sigma;
    sc_signal<int> delta;

    SC_CTOR(D) : in("in"), alpha("alpha"), beta("beta")
    {
        alpha.out(sigma);
        beta.in(sigma);
    }
};

SC_MODULE(top)
{
       A a;
       B b;
       D d;

       sc_signal<int> sig;

       SC_CTOR(top) : a("a"), b("b"), d("d")
       {
           a.out(sig);
           b.in(sig);
           d.in(sig);
       }
};

int sc_main(int argc, char *argv[])
{
    top t("top");
    visualize v; // Just instantiate here

    sc_start();
}
