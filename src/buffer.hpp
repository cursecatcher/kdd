/*
Copyright (c) 2020

GRAPES is provided under the terms of The MIT License (MIT):

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef BUFFER_HPP
#define BUFFER_HPP

#include <iostream>
#include <vector>
#include <meddly.h>
#include <meddly_expert.h>


using int_vector = std::vector<int>;
using buffer_slot = int_vector*;

/** Buffer for the data of a single graph. */
class Buffer : private std::vector<int_vector> { 
    using base = std::vector<int_vector>; 
    using value_vector = std::vector<long>; //template? 

    //iterator to the current buffer 
    Buffer::iterator current; 
    
    //container for output values 
    value_vector values; 
    //iterator to the current return value 
    value_vector::iterator current_value; 
    //enable the use of values vector 
    const bool enable_values; 

    size_t num_current_elements; 

    std::vector<int*> pbuffer; 


    inline void buildNewEdge(MEDDLY::dd_edge& edge) {
        MEDDLY::forest *forest = edge.getForest(); 

        switch (forest->getRangeType()) {
            case MEDDLY::forest::BOOLEAN:   
                forest->createEdge(data(), num_elements(), edge); 
                break; 
            case MEDDLY::forest::INTEGER:  
            case MEDDLY::forest::REAL:
                forest->createEdge(data(), values_data(), num_elements(), edge);
                break; 
            default:
                throw std::logic_error("Unexpected MEDDLY::forest::RangeType.\n");
        } 
    }


public: 
    //size of an internal buffer 
    const size_t element_size; 

    //constructor for empty buffer
    inline Buffer(size_t elem_size, bool set_values); 
    //constructor for buffer of buffersize elements 
    inline Buffer(size_t buffersize, size_t elem_size, bool set_values);

    inline void flush();

    //flush buffer content to MDD
    void flush(MEDDLY::dd_edge& edge) {
        if (num_elements()) {
            MEDDLY::forest *forest = edge.getForest();
            MEDDLY::dd_edge tmp(forest);

            try {
                buildNewEdge(tmp);  //build temporary edge depending on the DD type 
                edge += tmp;        //merge the temporary edge to the main DD  (performing PLUS or UNION operation)
                tmp.clear();        //clear temporary edge 
                flush();            //clear buffer 
            } catch (MEDDLY::error& e) {
                std::cout << "Meddly error during UNION: " << e.getName() << std::endl; 
                throw MEDDLY::error(e); 
            }
        }
    }

    /** Save in slot the first element available of the buffer.
     * returns true if there are other elements avaiable */
    inline bool get_slot(buffer_slot& slot);
    inline int_vector& push_slot(int value);

    inline Buffer::base::iterator begin() { return base::begin(); }
    inline Buffer::base::iterator end() { return base::end(); }

    inline void save_value(const long v);

    inline int** data() { return pbuffer.data(); }

    inline size_t size() { return base::size(); }

    inline long* values_data() {
        return enable_values ? values.data() : nullptr; 
    }

    inline unsigned num_elements() const {
        return num_current_elements;
    }

    inline void show_content() const; 
}; 


Buffer::Buffer(size_t elem_size, bool set_values)
    :   enable_values(set_values),  
        num_current_elements(0), 
        element_size(elem_size) {
}

Buffer::Buffer(size_t buffersize, size_t elem_size, bool set_values) 
    : Buffer(elem_size, set_values) {

    resize(buffersize); 

    if (enable_values) {
        values.resize(buffersize); 
        current_value = values.begin();
    } 

    for (auto it = begin(); it != end(); ++it) {
        it->resize(element_size); 
        pbuffer.push_back(it->data());
    }

    current = begin();  
}


inline void Buffer::flush() {
    current = begin(); 
    num_current_elements = 0; 

    if (enable_values)
        current_value = values.begin(); 
}


inline bool Buffer::get_slot(buffer_slot& slot) {
    slot = std::addressof(*current);
    ++num_current_elements; 

    if (++current == end()) {
        current = begin(); 
        return false; 
    }

    return true; 
}


inline int_vector& Buffer::push_slot(int value) {
    //append a new int_vector of length=element_size to the buffer
    emplace_back(element_size);
    int_vector& last_vector = back(); 
    pbuffer.push_back(last_vector.data()); 
    values.push_back(value); 
    ++num_current_elements; 
    return last_vector; 
}

inline void Buffer::save_value(const long v) {
    if (enable_values) {
        *current_value = v; 

        if (++current_value == values.end())
            current_value = values.begin(); 
    }
}

inline void Buffer::show_content() const {
    for (int i = 0; i < num_current_elements; ++i) {
        // const int_vector& v = at(i); 

        for (int val: at(i))
            std::cout << val << " ";
        if (enable_values)
            std::cout << "  ==> " << values.at(i); 
        std::cout << "\n"; 
    }
    std::cout << std::endl; 
}
#endif