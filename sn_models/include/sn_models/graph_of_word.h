#ifndef GRAPH_OF_WORD_H
#define GRAPH_OF_WORD_H

#include <sn_graph/sn_graph.hh>
#include <ros/ros.h>
#include <sn_dictionary/dico.h>

namespace sn {

typedef SimpleNode<Word> node_t;
typedef SimpleEdge<Word, false> edge_t;
typedef Graph<node_t, edge_t> GraphOfWordData;
typedef std::shared_ptr<GraphOfWordData> GraphOfWordDataPtr;

struct GraphOfWord{
    typedef std::shared_ptr<GraphOfWord> Ptr;

    GraphOfWord();

    std::string name;
    ros::Time uid;

    GraphOfWordDataPtr graph;

    void add(Word const& w);

    static Ptr create();

    void print()const;
};

typedef GraphOfWord::Ptr GraphOfWordPtr;

}

#endif // GRAPH_OF_WORD_H
