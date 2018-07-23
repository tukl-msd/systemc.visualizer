#ifndef VISUALIZE_H
#define VISUALIZE_H
#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include "systemc.h"
#include <regex>

class visualize
{
    public:

    visualize(bool debug=false) : debug(debug)
    {
        std::string filename = "graph.dot";
        f.open (filename);

        level = 0;
        f << "graph G {" << std::endl;

        std::vector<sc_object*> tops = sc_get_top_level_objects();
        for(auto t: tops)
        {
            if(std::string(t->kind()) == "sc_module")
            {
                if(debug)
                {
                    std::cout << t->name() << "(" << t->kind() << ")" << std::endl;
                }
                f << "subgraph cluster" << format(t->name()) << " {" << std::endl;
                f << "    label=\"" << t->basename() << "\";" << std::endl;
                f << "    shape=component;" << std::endl;
            }

            scanModule(*t);

            if(std::string(t->kind()) == "sc_module")
            {
                f << "}" << std::endl;
            }
        }
    }

    visualize(sc_module &t, bool debug = false) : debug(debug)
    {
        std::string filename = "graph.dot";
        f.open (filename);

        level = 0;
        f << "graph G {" << std::endl;

        if(debug)
        {
            std::cout << t.name() << "(" << t.kind() << ")" << std::endl;
        }
        f << "subgraph cluster" << format(t.name()) << " {" << std::endl;
        f << "    label=\"" << t.basename() << "\";" << std::endl;
        f << "    shape=component;" << std::endl;

        scanModule(t);

        f << "}" << std::endl;
    }

    ~visualize()
    {
        // Draw all edges between ports, which were collected in the edge map:
        for (auto const& x : edges)
        {
            if(x.second.size() == 2)
            {
                f << "    " << x.second[0]
                  << " -- " << x.second[1]
                  << ";" << std::endl;
            }
            else
            {
                f << (uint64_t)x.first
                  << " [shape=point];"
                  << std::endl;

                for (auto const& y : x.second)
                {
                    f << (uint64_t)x.first
                      << " -- " << y
                      << ";" << std::endl;
                }
            }
        }
        f << "}" << std::endl;
        f.close();

        std::cout << "VISUALIZE: graph.dot file written" << std::endl;

        // Only works on Linux und MAC
        system("dot -Tpdf graph.dot > graph.pdf");
    }

    private:

    ofstream f;

    // Used for proper indention of the DOT file:
    unsigned int level;

    // A map for collecting edges between sc_ports
    std::map<sc_interface *, std::vector<std::string>> edges;

    bool debug;

    void scanModule(sc_object &t)
    {
        level++;

        for(auto child : t.get_child_objects())
        {
            if(debug)
            {
                indent(std::cout, level);
                std::cout << child->name()
                          << "(" << child->kind() << ")" << std::endl;
            }

            // If port was found:
            if(std::string(child->kind()) == "sc_in" ||
               std::string(child->kind()) == "sc_out" ||
               std::string(child->kind()) == "sc_fifo_in" ||
               std::string(child->kind()) == "sc_fifo_out" ||
               std::string(child->kind()) == "tlm_target_socket" ||
               std::string(child->kind()) == "sc_port")
            {
                sc_port_base * optr = dynamic_cast<sc_port_base*>(child);
                sc_interface * pointer = optr->get_interface();

                indent(f, level);
                f << format(child->name())
                  << " [label=\"" << child->basename() << "\"]"
                  << ";" << std::endl;

                edges[pointer].push_back(format(child->name()));
            }
            else if(std::string(child->kind()) == "tlm_initiator_socket" || // TODO
                    std::string(child->kind()) == "sc_export" ) // TODO
            {
                // TODO does not work yet
            }
            else if(std::string(child->kind()) == "sc_module")
            {
                indent(f, level);
                f << "subgraph cluster" << format(child->name()) << " {" << std::endl;
                indent(f, level);
                f << "    label=\"" << child->basename() << "\";" << std::endl;
                indent(f, level);
                f << "    shape=component;" << std::endl;
            }


            // Analyze the Child:
            scanModule(*child);

            if(std::string(child->kind()) == "sc_module")
            {
                indent(f, level);
                f << "}" << std::endl;
            }
        }

        level--;
    }

    // Replace . by _ because DOT does not support .
    std::string format(std::string str)
    {
        return std::regex_replace(str, std::regex("\\."), "_" );
    }

    // Indention acording to the level:
    void indent(std::ostream &o, unsigned int level)
    {
        for(unsigned int i = 0; i < level; i++)
        {
            o << "    ";
        }
    }

};

#endif // VISUALIZE_H
