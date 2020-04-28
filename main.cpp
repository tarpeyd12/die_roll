#include <cassert>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <thread>
#include <chrono>
#include <algorithm>
#include <random>

#include "die_table.hpp"



void
print_table( const DieTable& table, std::ostream& out = std::cout )
{
    auto max = table.chance_total();
    table.for_each( [&out,&max]( auto face_value, const auto& prob_value )
    {
        out << "[" << face_value << "] = ";
        //out << prob_value << "/" << max << " ";
        out << _get_bigint_fraction( prob_value, max ) * 100.0L << "%";
        out << "\n";
        out << std::flush;
    } );

    out << std::flush;
}

DieTable
add_tables( const std::vector<DieTable>& tables )
{
    assert( !tables.empty() );

    DieTable out( tables[0] );
    for( size_t i = 1; i < tables.size(); ++i )
    {
        out = out.grok( tables[i], []( const auto& a, const auto& b ){ return a + b; } );
    }

    return out;
}

DieTable
maxof_tables( const std::vector<DieTable>& tables )
{
    assert( !tables.empty() );

    DieTable out( tables[0] );
    for( size_t i = 1; i < tables.size(); ++i )
    {
        out = out.grok( tables[i], []( const auto& a, const auto& b ){ return std::max( a, b ); } );
    }

    return out;
}

std::vector<DieTable>
duplicate_table( const DieTable& table, size_t count )
{
    return std::vector<DieTable>( count, table );
}

DieTable
loot( size_t num_players = 24, size_t num_drops = 3, size_t num_passes = 0 )
{
    const DieTable die( 1, 99 );
    // counts the number of times that your rolls are higher than the maximum roll of num_players-1 player's rolls, summed by num_drops. assumes tie-breakers are not in your favor
    return add_tables( duplicate_table( DieTable( die ).grok( maxof_tables( duplicate_table( DieTable( die ), num_players-1-num_passes ) ), []( const auto& a, const auto& b ){ return a>b?1:0; } ), num_drops ) );
}

DieTable
condence_non_zero( const DieTable& in_table )
{
    return DieTable( 0 ).grok( in_table, []( const auto& a, const auto& b ){ return a+b?1:0; } );
}

bool
roll_on_chest()
{
    thread_local std::mt19937_64 generator( (uint64_t)time( nullptr ) );

    /*
    {
        // numerically accurate way of calculating the chances, everything is ints
        thread_local std::uniform_int_distribution<int> distribution( 1, 99 );

        auto roll = [&]() -> int
        {
            return distribution( generator );
        };

        // roll for the 23 other players
        thread_local std::vector<int> other_rolls( 23, 0 );
        std::generate( other_rolls.begin(), other_rolls.end(), [&]{ return roll(); } );

        // compare our roll with that of the maximum of the other players
        return roll() > *std::max_element( other_rolls.begin(), other_rolls.end() );
    }
    */

    {
        // much faster way to calculate the chances, using floating point numbers
        thread_local long double chance = condence_non_zero( loot( 24, 1, 0 ) )[1]; // chance to get 1 chest, we only need to calculate this once
        thread_local std::uniform_real_distribution<long double> distribution( 0.0L, 1.0L );

        return distribution( generator ) <= chance;
    }

}

int main()
{
    std::cout.sync_with_stdio( false );
    std::cout << "worst chance to get the 2B outfit:" << std::endl;
    print_table( condence_non_zero( loot( 24, 3, 0 ) ) );

    std::cout << "\nworst chance to get the Pod mini:" << std::endl;
    print_table( loot( 24, 1, 0 ) );

    std::cout << "\nworst chance to get any 1 piece of gear:" << std::endl;
    print_table( loot( 8, 1, 0 ) );

    {
        std::ofstream csv("pass_chance.csv");
        csv << "passers,chance" << std::endl;
        std::cout << "\n";
        for( int i = 0; i < 24; ++i )
        {
            auto percent_chance = i==23?1.0L:condence_non_zero( loot( 24, 3, i ) )[1];
            csv << i << "," << std::setprecision( std::numeric_limits< long double >::max_digits10 + 2 ) << percent_chance << std::endl;
            std::cout << "chance of wining a chest when " << i << " other players pass: " << percent_chance*100.0L << "%" << std::endl;
        }
        csv.close();
        std::cout << "\n";
    }

    {
        std::cout << std::endl << std::endl;
        size_t rolls = 0;
        size_t acquired = 0;
        while( rolls < 1000000 )
        {
            if( roll_on_chest() || roll_on_chest() || roll_on_chest() )
            {
                ++acquired;
            }
            ++rolls;

            if(rolls%1000==0)
            std::cout << "ran raid " << rolls << " times, got chests " << acquired << " times. " << (long double)( acquired ) / (long double)( rolls ) * 100.0L << "%\r" << std::flush;
        }
    }

    {
        std::cout << std::endl << std::endl;
        size_t acquisitions = 0;
        std::map<size_t,size_t> histogram;
        size_t total_runs = 0;
        while( acquisitions < 1000000 )
        {
            size_t runs = 0;
            do
            {
                ++runs;
            }
            while( !( roll_on_chest() || roll_on_chest() || roll_on_chest() ) );
            histogram[runs]++;
            ++acquisitions;
            total_runs += runs;
            //std::cout << "acquired chest after " << runs << " runs \r" << std::flush;
            if(acquisitions%10000==0)
            std::cout << "acquired " << acquisitions << " chests over " << total_runs << " total runs. acquired chest after an average of " << std::setprecision(7) << std::setw(9) << std::right << std::fixed << (long double)(total_runs)/(long double)(acquisitions) << " runs\r" << std::flush;
        }

        std::cout << std::endl;


        std::ofstream csv("out.csv");
        csv << "runs,times" << std::endl;

        std::cout << std::endl;
        size_t sum = 0;
        for( auto& h : histogram )
        {
            sum += h.second;
            std::cout << std::setw(4) << std::right << h.first << " runs, " << std::setw(10) << std::right << h.second << " times. ";
            std::cout << std::setprecision(6) << std::setw(9) << std::right << std::fixed << (long double)(h.second)/(long double)(acquisitions)*100.0L << "% of people acquired a chest with this many runs. ";
            std::cout << std::setprecision(5) << std::setw(9) << std::right << std::fixed << (long double)(sum)/(long double)(acquisitions)*100.0L << "% of acquisitions occur by this point\n";

            csv << h.first << "," << h.second << std::endl;
        }
        std::cout << std::flush;

        csv << std::flush;
        csv.close();
    }

    std::cout << "\a" << std::flush;
    std::this_thread::sleep_for( std::chrono::duration<double>(0.75) );
    std::cout << "\a" << std::flush;

    return 0;
}
