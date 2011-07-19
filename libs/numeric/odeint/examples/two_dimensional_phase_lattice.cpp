/*
 * two_dimensional_phase_lattice.cpp
 *
 * This example show how one can use matrices as state types in odeint.
 *
 *  Created on: Jul 15, 2011
 *      Author: karsten
 *
 * Copyright 2009 Karsten Ahnert and Mario Mulansky.
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

#include <iostream>
#include <map>
#include <string>
#include <fstream>

#include <boost/numeric/odeint.hpp>

#include <boost/numeric/odeint/util/ublas_wrapper.hpp>


using namespace std;
using namespace boost::numeric::odeint;

//[ two_dimensional_phase_lattice_definition
typedef boost::numeric::ublas::matrix< double > state_type;

struct two_dimensional_phase_lattice
{
    two_dimensional_phase_lattice( double gamma = 0.5 )
    : m_gamma( gamma ) { }

    void operator()( const state_type &x , state_type &dxdt , double t ) const
    {
        size_t size1 = x.size1() , size2 = x.size2();

        for( size_t i=1 ; i<size1-1 ; ++i )
        {
            for( size_t j=1 ; j<size2-1 ; ++j )
            {
                dxdt( i , j ) =
                        coupling_func( x( i + 1 , j ) - x( i , j ) ) +
                        coupling_func( x( i - 1 , j ) - x( i , j ) ) +
                        coupling_func( x( i , j + 1 ) - x( i , j ) ) +
                        coupling_func( x( i , j - 1 ) - x( i , j ) );
            }
        }

        for( size_t i=0 ; i<x.size1() ; ++i ) dxdt( i , 0 ) = dxdt( i , x.size2() -1 ) = 0.0;
        for( size_t j=0 ; j<x.size2() ; ++j ) dxdt( 0 , j ) = dxdt( 0 , x.size1() -1 ) = 0.0;
    }

    double coupling_func( double x ) const
    {
        return sin( x ) - m_gamma * ( 1.0 - cos( x ) );
    }

    double m_gamma;
};
//]


struct write_for_gnuplot
{
    size_t m_every , m_count;

    write_for_gnuplot( size_t every = 10 )
    : m_every( every ) , m_count( 0 ) { }

    void operator()( const state_type &x , double t )
    {
        if( ( m_count % m_every ) == 0 )
        {
            cout << "sp '-'" << endl;
            for( size_t i=0 ; i<x.size1() ; ++i )
            {
                for( size_t j=0 ; j<x.size2() ; ++j )
                {
                    cout << i << "\t" << j << "\t" << sin( x( i , j ) ) << "\n";
                }
                cout << "\n";
            }
            cout << "e" << endl;
        }

        ++m_count;
    }
};

class write_snapshots
{
public:

    typedef std::map< size_t , std::string > map_type;

    write_snapshots( void ) : m_count( 0 ) { }

    void operator()( const state_type &x , double t )
    {
        map< size_t , string >::const_iterator it = m_snapshots.find( m_count );
        if( it != m_snapshots.end() )
        {
            ofstream fout( it->second.c_str() );
            for( size_t i=0 ; i<x.size1() ; ++i )
            {
                for( size_t j=0 ; j<x.size2() ; ++j )
                {
                    fout << i << "\t" << j << "\t" << x( i , j ) << "\t" << sin( x( i , j ) ) << "\n";
                }
                fout << "\n";
            }
        }
        ++m_count;
    }

    map_type& snapshots( void ) { return m_snapshots; }
    const map_type& snapshots( void ) const { return m_snapshots; }

private:

    size_t m_count;
    map_type m_snapshots;
};


int main( int argc , char **argv )
{
    size_t size1 = 128 , size2 = 128;
    state_type x( size1 , size2 , 0.0 );

    for( size_t i=(size1/2-10) ; i<(size1/2+10) ; ++i )
        for( size_t j=(size2/2-10) ; j<(size2/2+10) ; ++j )
            x( i , j ) = drand48() * 2.0 * M_PI;

    write_snapshots snapshots;
    snapshots.snapshots().insert( make_pair( size_t( 0 ) , string( "lat_0000.dat" ) ) );
    snapshots.snapshots().insert( make_pair( size_t( 100 ) , string( "lat_0100.dat" ) ) );
    snapshots.snapshots().insert( make_pair( size_t( 1000 ) , string( "lat_1000.dat" ) ) );
    observer_collection< state_type , double > obs;
    obs.observers().push_back( write_for_gnuplot( 1 ) );
    obs.observers().push_back( snapshots );

    cout << "set term x11" << endl;
    cout << "set pm3d map" << endl;
    integrate_const( runge_kutta_fehlberg4< state_type >() , two_dimensional_phase_lattice( 1.2 ) ,
            x , 0.0 , 101.0 , 0.1 , boost::ref( snapshots ) );


    return 0;
}
