//
// Original stock.hpp
// Modified eye_message.hpp
// ~~~~~~~~~
//
// Original Work Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
// Modified Work Copyright 2018 Cody Feltch
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef SERIALIZATION_STOCK_HPP
#define SERIALIZATION_STOCK_HPP

#include <string>

namespace codechallenge
{

/// Structure to hold information about a single stock.
struct eye_message {

    ulong seq_number;
    uint64_t time_seconds;
    uint32_t time_millis;
    bool id;
    float confidence;
    float normalized_pos_x;
    float normalized_pos_y;
    uint32_t pupil_diameter;


    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar &  seq_number;
        ar &  time_seconds;
        ar &  time_millis;
        ar &  id;
        ar &  confidence;
        ar &  normalized_pos_x;
        ar &  normalized_pos_y;
        ar &  pupil_diameter;
    }
};

} // namespace codechallenge

#endif // SERIALIZATION_STOCK_HPP
