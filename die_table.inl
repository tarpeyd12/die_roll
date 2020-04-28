#ifndef DIE_TABLE_INL_INCLUDED
#define DIE_TABLE_INL_INCLUDED

#include <cassert>
#include <numeric>
#include <algorithm>
#include <sstream>

DieTable::DieTable()
    : m_dieFaceMap()
{
    /*  */
}

DieTable::DieTable( int64_t value )
    : DieTable( value, value )
{
    /*  */
}

DieTable::DieTable( int64_t min, int64_t max )
    : m_dieFaceMap()
{
    if( min > max )
    {
        std::swap( min, max );
    }

    for( int64_t i = min; i <= max; ++i )
    {
        m_dieFaceMap[ i ] = 1;
    }
}

/*DieTable::DieTable( const std::string& str )
{

}*/

DieTable::DieTable( const std::map<int64_t,BigInt>& table )
    : m_dieFaceMap( table )
{
    /*  */
}

DieTable::DieTable( const DieTable& table )
    : DieTable( table.m_dieFaceMap )
{
    /*  */
}

int64_t
DieTable::min_value() const
{
    return std::min_element( m_dieFaceMap.begin(), m_dieFaceMap.end(), [](const auto& l, const auto& r) { return l.first < r.first; } )->first;
}

int64_t
DieTable::max_value() const
{
    return std::max_element( m_dieFaceMap.begin(), m_dieFaceMap.end(), [](const auto& l, const auto& r) { return l.first < r.first; } )->first;
}

BigInt
DieTable::min_chance() const
{
    return std::min_element( m_dieFaceMap.begin(), m_dieFaceMap.end(), [](const auto& l, const auto& r) { return l.second < r.second; } )->second;
}

BigInt
DieTable::max_chance() const
{
    return std::max_element( m_dieFaceMap.begin(), m_dieFaceMap.end(), [](const auto& l, const auto& r) { return l.second < r.second; } )->second;
}

template< typename T >
std::string
_to_string( const T& value )
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

template< typename T >
T
_from_string( const std::string& str )
{
    std::istringstream iss( str );
    T value{};
    iss >> value;
    return value;
}

long double
_get_bigint_fraction( const BigInt& numerator, const BigInt& denominator )
{
    long double out = 0.0L;
    try
    {
        out = (long double)(numerator.to_long_long()) / (long double)(denominator.to_long_long());
    }
    catch( std::out_of_range )
    {
        std::string numer_str = _to_string( numerator );
        std::string denom_str = _to_string( denominator );

        // so converting to long double's via int64_t's can't hold the actual values, so we convert to long double's via stings now!
        out = _from_string<long double>( numer_str ) / _from_string<long double>( denom_str );

        // if we get some nan's or inf's
        if( !std::isnormal( out ) && out != 0.0L )
        {
            // ok, so this is really hackish,
            // so we basically reduce the length of the intermediate strings until long double can hold the result and not be bad
            // and we are only really losing precision that a long double couldn't hold anyway
            long double attempt = out;
            do
            {
                while( numer_str.size() > 200 && denom_str.size() > 200 )
                {
                    numer_str = numer_str.substr( 0, numer_str.size() - 25 );
                    denom_str = denom_str.substr( 0, denom_str.size() - 25 );
                }
                //else
                {
                    numer_str = numer_str.substr( 0, numer_str.size() - 1 );
                    denom_str = denom_str.substr( 0, denom_str.size() - 1 );
                }

                attempt = _from_string<long double>( numer_str ) / _from_string<long double>( denom_str );
            }
            while( !std::isnormal( attempt ) && attempt != 0.0L && numer_str.size() > (numer_str[0]=='-'?2:1) && denom_str.size() > (denom_str[0]=='-'?2:1) );

            out = attempt;
        }
    }
    catch( ... )
    {
        /*  */
    }

    return out;
}

long double
DieTable::percent_chance_of( int64_t face_key ) const
{
    BigInt numerator   = chance_of( face_key );
    BigInt denominator = chance_total();

    return _get_bigint_fraction( numerator, denominator );
}

BigInt
DieTable::chance_of( int64_t face_key ) const
{
    auto search = m_dieFaceMap.find( face_key );
    return search == m_dieFaceMap.end() ? 0 : search->second;
}

BigInt
DieTable::chance_total() const
{
    return std::accumulate( m_dieFaceMap.begin(), m_dieFaceMap.end(), BigInt(0), []( const auto& a, const auto& b ){ return a + b.second; } );
}

long double
DieTable::operator[]( int64_t face_key ) const
{
    return percent_chance_of( face_key );
}

DieTable
DieTable::grok( const DieTable& other, const std::function<int64_t(int64_t,int64_t)>& func ) const
{
    DieTable out; // cannot use most functions on this

    for( auto this_values : m_dieFaceMap )
    {
        const int64_t& a = this_values.first;
        const BigInt& a_prob = this_values.second;

        for( auto other_values : other.m_dieFaceMap )
        {
            const int64_t& b = other_values.first;
            const BigInt& b_prob = other_values.second;

            out.m_dieFaceMap[ func( a, b ) ] += a_prob * b_prob;
        }
    }

    return out;
}

void
DieTable::for_each( const std::function< void(int64_t,const BigInt&) >& func ) const
{
    for( auto this_values : m_dieFaceMap )
    {
        func( this_values.first, this_values.second );
    }
}

#endif // DIE_TABLE_INL_INCLUDED
