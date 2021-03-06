/******************************************************************************
 * \file
 * $Id: but_server.cpp 809 2012-05-19 21:47:48Z stancl $
 *
 * Modified by dcgm-robotics@FIT group.
 *
 * Author: Vit Stancl (stancl@fit.vutbr.cz)
 * Supervised by: Michal Spanel (spanel@fit.vutbr.cz)
 * Date: dd.mm.2011
 *
 * This code is derived from the OctoMap server provided by A. Hornung.
 * Please, see the original comments below.
 */

/**
 * octomap_server: A Tool to serve 3D OctoMaps in ROS (binary and as visualization)
 * (inspired by the ROS map_saver)
 * @author A. Hornung, University of Freiburg, Copyright (C) 2010-2011.
 * @see http://octomap.sourceforge.net/
 * License: BSD
 */

/**
 * Copyright (c) 2010-2011, A. Hornung, University of Freiburg
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the University of Freiburg nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <srs_env_model/but_server/but_server.h>
#include <srs_env_model/topics_list.h>
//#include <srs_env_model/but_server/server_tools.h>

#include <sstream>

//! Swap function template
//template <typename tpType> void swap( tpType & x, tpType & y){ tpType b(x); x = y; y = b; }

///////////////////////////////////////////////////////////////////////////////
/**
 Constructor
 */
srs_env_model::CButServer::CButServer(const std::string& filename) :
			m_bIsPaused(false),
			m_nh(),
			m_latchedTopics(false),
			m_numPCFramesProcessed(1.0), m_frameCounter(0),
			m_plugCMapHolder("CMAP"),
			m_plugInputPointCloudHolder("PCIN"),
			m_plugOcMapPointCloudHolder("PCOC"),
			m_plugVisiblePointCloudHolder("PCVIS"),
			m_plugOctoMap("OCM"),
			m_plugCollisionObjectHolder("COB"),
			m_plugMap2DHolder("M2D"),
			m_plugIMarkers(0),
			m_plugMarkerArrayHolder( "MA" ),
			m_plugObjTree( "OT" ),
			m_plugOldIMarkers( 0 ),
			m_bUseOldIMP( false )
#ifdef _EXAMPLES_
			, m_plugExample("Example1")
			, m_plugExampleCrawlerHolder( "Example2")
#endif

{
	// Get node handle
	ros::NodeHandle private_nh("~");

	// Advertise services
	m_serviceReset = private_nh.advertiseService(ServerReset_SRV, &CButServer::onReset, this);
	m_servicePause = private_nh.advertiseService(ServerPause_SRV, &CButServer::onPause, this);

	// Is map static (loaded from file)?
	bool staticMap(filename != "");

	m_latchedTopics = staticMap;
	private_nh.param("latch", m_latchedTopics, m_latchedTopics);
	private_nh.param<bool>("use_old_im", m_bUseOldIMP, m_bUseOldIMP);

	std::cerr << "BUTSERVER: Initializing plugins " << std::endl;

	// Store all plugins pointers for easier access
	m_plugins.push_back( m_plugCMapHolder.getPlugin() );
	m_plugins.push_back( m_plugInputPointCloudHolder.getPlugin() );
	m_plugins.push_back( m_plugOcMapPointCloudHolder.getPlugin() );
	m_plugins.push_back( m_plugVisiblePointCloudHolder.getPlugin() );
	m_plugins.push_back( &m_plugOctoMap );
	m_plugins.push_back( m_plugCollisionObjectHolder.getPlugin() );
	m_plugins.push_back( m_plugMap2DHolder.getPlugin() );
	m_plugins.push_back( m_plugMarkerArrayHolder.getPlugin() );
	m_plugins.push_back( &m_plugObjTree );

	if( m_bUseOldIMP )
	{
		m_plugOldIMarkers = new COldIMarkersPlugin( "IM" );
		m_plugins.push_back( m_plugOldIMarkers );
	}
	else
	{
		m_plugIMarkers = new CIMarkersPlugin( "IM" );
		m_plugins.push_back( m_plugIMarkers );
	}

#ifdef _EXAMPLES_
	m_plugins.push_back( & m_plugExample );
	m_plugins.push_back( m_plugExampleCrawlerHolder.getPlugin() );
#endif


	//=========================================================================
	// Initialize plugins
	FOR_ALL_PLUGINS_PARAM(init, private_nh)

	std::cerr << "BUTSERVER: All plugins initialized. Starting server. " << std::endl;

	// Connect input point cloud input with octomap
	m_plugInputPointCloudHolder.getPlugin()->getSigDataChanged().connect( boost::bind( &COctoMapPlugin::insertCloud, &m_plugOctoMap, _1 ));

	// Connect octomap data changed signal with server publish
	m_plugOctoMap.getSigDataChanged().connect( boost::bind( &CButServer::onOcMapDataChanged, this, _1 ));


} // Constructor

///////////////////////////////////////////////////////////////////////////////
/**
 Destructor
 */
srs_env_model::CButServer::~CButServer()
{

	if( m_plugOldIMarkers != 0 )
		delete m_plugOldIMarkers;

	if( m_plugIMarkers != 0 )
		delete m_plugIMarkers;

}



///////////////////////////////////////////////////////////////////////////////

/**
 Publish all data
 */
void srs_env_model::CButServer::publishAll(const ros::Time& rostime) {

	// Store start time
	ros::WallTime startTime = ros::WallTime::now();

	// If no data, do nothing
	if (m_plugOctoMap.getSize() <= 1) {
		ROS_WARN("Nothing to publish, octree is empty");
		return;
	}


	// init markers:
	visualization_msgs::MarkerArray occupiedNodesVis;

	// each array stores all cubes of a different size, one for each depth level:
	occupiedNodesVis.markers.resize(m_plugOctoMap.getTreeDepth() + 1);

	//=========================================================================
	// Plugins frame start

	m_plugOcMapPointCloudHolder.connect( & m_plugOctoMap );
	m_plugCollisionObjectHolder.connect( & m_plugOctoMap );
	m_plugCMapHolder.connect( & m_plugOctoMap );
	m_plugMap2DHolder.connect( & m_plugOctoMap );
    m_plugMarkerArrayHolder.connect( & m_plugOctoMap );
    m_plugVisiblePointCloudHolder.connect( & m_plugOctoMap );

#ifdef _EXAMPLES_
    m_plugExampleCrawlerHolder.connect( & m_plugOctoMap );
#endif

	// Crawl octomap
	m_plugOctoMap.crawl( rostime );

#ifdef _EXAMPLES_
	m_plugExampleCrawlerHolder.disconnect();
#endif

	// Disconnect all
	m_plugOcMapPointCloudHolder.disconnect();
    m_plugCollisionObjectHolder.disconnect();
	m_plugCMapHolder.disconnect();
	m_plugMap2DHolder.disconnect();
	m_plugMarkerArrayHolder.disconnect();
	m_plugVisiblePointCloudHolder.disconnect();

   // Publish point cloud
    m_plugOcMapPointCloudHolder.publish( rostime );

	// Publish collision object
	m_plugCollisionObjectHolder.publish( rostime );

	// Publish collision map
	m_plugCMapHolder.publish( rostime );

	// Publish 2D map
	m_plugMap2DHolder.publish( rostime );

	// Finalize octomap publishing
	if (m_plugOctoMap.shouldPublish())
		m_plugOctoMap.onPublish( rostime );

	m_plugMarkerArrayHolder.publish(rostime);

	// Publish pointcloud visible in rviz
	m_plugVisiblePointCloudHolder.publish(rostime);

	// Compute and show elapsed time
	double total_elapsed = (ros::WallTime::now() - startTime).toSec();
	ROS_DEBUG("Map publishing in CButServer took %f sec", total_elapsed);

	// Publish interactive markers
	if( m_plugIMarkers != 0 && m_plugIMarkers->shouldPublish() )
		m_plugIMarkers->onPublish( rostime );

	// Old interactive markers
	if( m_plugOldIMarkers != 0 && m_plugOldIMarkers->shouldPublish() )
		m_plugOldIMarkers->onPublish( rostime );

#ifdef _EXAMPLES_
	// Publish data
	if( m_plugExample.shouldPublish() )
	  m_plugExample.onPublish(rostime);

	m_plugExampleCrawlerHolder.publish(rostime);
#endif
}


/**
 * On octomap data changed
 */
void srs_env_model::CButServer::onOcMapDataChanged( const tButServerOcMap & mapdata )
{
	// Publish all data
	publishAll(ros::Time::now() );
}

/**
 * @brief Reset server and all plugins.
 */
void srs_env_model::CButServer::reset()
{
  ROS_DEBUG("Reseting environment server...");

  FOR_ALL_PLUGINS(reset());

  ROS_DEBUG("Environment server reset finished.");

}

/**
 * On pause service call
 */
bool srs_env_model::CButServer::onPause( ButServerPause::Request & request, ButServerPause::Response & response )
{
	if( request.pause == 0 )
		pause( false );
	else
		pause( true );
}

/**
 * Pause-resume server
 */
void srs_env_model::CButServer::pause( bool bPause )
{
	if( m_bIsPaused == bPause )
		return;

	// Get node handle
	ros::NodeHandle private_nh("~");

	tVecPlugins::iterator p, end( m_plugins.end() );

	for( p = m_plugins.begin(); p != end; ++p )
	{
		(*p)->pause( bPause, private_nh );
	}

	if( bPause )
		std::cerr << "BUT server paused..." << std::endl;
	else
		std::cerr << "BUT server resumed..." << std::endl;

	m_bIsPaused = bPause;
}

