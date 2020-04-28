#ifndef DIE_TABLE_HPP_INCLUDED
#define DIE_TABLE_HPP_INCLUDED

#include <map>
#include <functional>

#include "BigInt.hpp"

class DieTable
{
    public:

        DieTable( int64_t value );
        DieTable( int64_t min, int64_t max );
        //DieTable( const std::string& str );
        explicit DieTable( const std::map<int64_t,BigInt>& table );
        DieTable( const DieTable& table );

        int64_t min_value() const;
        int64_t max_value() const;

        BigInt min_chance() const;
        BigInt max_chance() const;

        long double percent_chance_of( int64_t face_key ) const;
        BigInt chance_of( int64_t face_key ) const;
        BigInt chance_total() const;

        long double operator[]( int64_t face_key ) const;

        DieTable grok( const DieTable& other, const std::function<int64_t(int64_t,int64_t)>& func ) const;

        void for_each( const std::function< void(int64_t,const BigInt&) >& func ) const;

    private:

        DieTable();

        std::map<int64_t,BigInt> m_dieFaceMap;
};

#include "die_table.inl"

#endif // DIE_TABLE_HPP_INCLUDED
