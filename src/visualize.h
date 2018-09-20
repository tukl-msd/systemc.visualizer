#ifndef VISUALIZE_H
#define VISUALIZE_H
#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include "systemc.h"
#include <regex>

class Port
{
    public:
    std::string id;
    std::string name;
    sc_interface * ptr;

    Port(std::string id, std::string name) : id(id), name(name)
    {
    }
};

//class Edge
//{
//    Port start;
//    Port end;
//};

class Module
{
    public:
    std::string id;
    std::string name;

    Module(std::string id, std::string name) : id(id), name(name)
    {
    }

    Module()
    {
    }

    std::vector<Module> submodules;
    std::vector<Port> ports;
    std::map<sc_interface *, std::vector<std::string>> edges;
};

class visualize
{
    private:

    ofstream f;
    ofstream e;

    // Used for proper indention of the DOT file:
    unsigned int level;

    // A map for collecting edges between sc_ports
    std::map<sc_interface *, std::vector<std::string>> edges;

    bool debug;

    std::vector<Module> topModules;

    public:

    visualize(bool debug=false) : debug(debug)
    {
        std::string dot = "graph.dot";
        std::string elk = "graph.json";
        f.open (dot);
        e.open (elk);

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
                topModules.push_back(Module(format(t->name()), t->basename()));

                scanModule(*t, &topModules.back());
            }


            if(std::string(t->kind()) == "sc_module")
            {
                f << "}" << std::endl;
            }
        }
    }

    visualize(sc_module &t, bool debug = false) : debug(debug)
    {
        std::string dot = "graph.dot";
        f.open(dot);

        level = 0;
        f << "graph G {" << std::endl;

        if(debug)
        {
            std::cout << t.name() << "(" << t.kind() << ")" << std::endl;
        }
        f << "subgraph cluster" << format(t.name()) << " {" << std::endl;
        f << "    label=\"" << t.basename() << "\";" << std::endl;
        f << "    shape=component;" << std::endl;

        scanModule(t, 0); // TODO

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

        level = 0;
        e << "algorithm: layered" << std::endl;
        generateElk(topModules);

        e.close();

        std::cout << "VISUALIZE: graph.json file written" << std::endl;

        // Only works on Linux und MAC
        system("dot -Tpdf graph.dot > graph.pdf");
    }

    void generateElk(std::vector<Module> modules)
    {
        for(auto module : modules)
        {
            std::cout << module.name << std::endl;

            indent(e,level);
            e << "node " << module.id << std::endl;
            indent(e,level);
            e << "{ " << std::endl;
            indent(e,level+1);
            e <<  "nodeLabelPlacement: \"OUTSIDE H_CENTER V_BOTTOM\"" << std::endl;
            indent(e,level+1);
            e << "portLabelPlacement: OUTSIDE" << std::endl;
            indent(e,level+1);
            e << "label \"" << module.name << "\"" << std::endl;

            for(auto port : module.ports)
            {
                std::cout << "    " << port.name << std::endl;
                indent(e,level+1);
                e << "port " << port.id << " { label '" << port.name << "' }" << std::endl;
            }

            level ++;
            generateElk(module.submodules);
            level --;
            indent(e,level);
            e << "}" << std::endl;
        }
    }

    void scanModule(sc_object &t, Module * currentModule)
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

            if(std::string(child->kind()) == "sc_module")
            {
                indent(f, level);
                f << "subgraph cluster" << format(child->name()) << " {" << std::endl;
                indent(f, level);
                f << "    label=\"" << child->basename() << "\";" << std::endl;
                indent(f, level);
                f << "    shape=component;" << std::endl;

                currentModule->submodules.push_back(Module(format(child->name()),child->basename()));

                // Analyze the Child:
                scanModule(*child, & currentModule->submodules.back());
            }
            else // If port was found:
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

                //edges[pointer].push_back(format(child->name()));
                currentModule->edges[pointer].push_back(format(child->name()));

                currentModule->ports.push_back(Port(format(child->name()),
                                                    child->basename()));
            }
            else if(std::string(child->kind()) == "tlm_initiator_socket" || // TODO
                    std::string(child->kind()) == "sc_export" ) // TODO
            {
                // TODO does not work yet
            }

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
