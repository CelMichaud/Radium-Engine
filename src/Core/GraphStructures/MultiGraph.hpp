#ifndef RADIUMENGINE_MULTIGRAPH_H
#define RADIUMENGINE_MULTIGRAPH_H

#include <algorithm>
#include <memory>
#include <vector>
#include <map>

namespace Ra
{
    namespace Core
    {

        /// @brief MultiGraph is an oriented and acyclic graph
        /// whose edges are represented by each node knowing the whole
        /// list of its parents.
        /// @note Usually the operators assume there is a unique source
        template <typename T>
        class MultiGraph
        {

            class Node
            {
                /// this is used to represent a connection from the source's m_slot to
                /// the local slot m_local
                struct Connection
                {
                    uint  m_slot;
                    Node* m_source;
                    uint  m_local;
                    bool  operator==(const Connection& other);
                };

            public:
                Node(const std::string& name, const std::shared_ptr<T>& data, uint nbIn, uint nbOut);
                ~Node() {}

                /// connect the local slot to other's slot
                void setParent(uint local, Node* other, uint slot);

                /// disconnect other's slot from local if it was previously connected
                void removeParent(uint local, Node* other, uint slot);

                /// update the level attribute of this if parents are all positively levelled
                void updateLevel();

                /// debug indicator - TODO(hugo) remove this
                void print();

            public:
                std::shared_ptr<T>      m_data;    /// data stored by the node
                std::vector<Connection> m_parents; /// parents of the node
                std::string m_name;
                uint m_level;
                uint m_nbIn;
                uint m_nbOut;
            }; // end of subclass Node

        public:
            MultiGraph() {}
            MultiGraph(const MultiGraph& other)
                : m_graph(other.m_graph)
                , m_names (other.m_names)
            {}
            ~MultiGraph() {}

            /// add a node to the graph
            void addNode(const std::string& name, const std::shared_ptr<T>& data, uint nb_in, uint nb_out);

            /// update the nodes' levels taking in account every source
            void levelize(bool sortByLevel = false);

            /// debug indicator - TODO(hugo) remove this
            void print();

            /// access a node pointer directly through the graph
            Node* operator[](const std::string& name);

            /// redefine iterator begin to map over m_graph
            typename std::vector<std::unique_ptr<Node>>::iterator begin();

            /// redefine iterator end to map over m_graph
            typename std::vector<std::unique_ptr<Node>>::iterator end();

        private:
            std::vector<std::unique_ptr<Node>> m_graph;
            std::map<std::string, Node*> m_names;
        }; // end of class MultiGraph

        // TODO(hugo) ask for preference about inline / this solution
        #include "MultiGraph.tpp"

    }
}

#endif // RADIUMENGINE_MULTIGRAPH_H
