/******************************************************************************
 * \file
 *
 * $Id:$
 *
 * Copyright (C) Brno University of Technology
 *
 * This file is part of software developed by dcgm-robotics@FIT group.
 *
 * Author: Vit Stancl (stancl@fit.vutbr.cz)
 * Supervised by: Michal Spanel (spanel@fit.vutbr.cz)
 * Date: 5/4/2012
 * 
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this file.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "rviz/plugin/type_registry.h"

#include "srs_assisted_arm_navigation/rviz_plugins/arm_navigation_display.h"
#include "srs_assisted_arm_navigation/rviz_plugins/grasping_display.h"
#include "srs_assisted_arm_navigation/rviz_plugins/bb_estimation_display.h"

using namespace srs_assisted_arm_navigation;

extern "C" void rvizPluginInit(rviz::TypeRegistry* reg)
{
    reg->registerDisplay<CButArmNavDisplay> ("CButArmNavDisplay");
    reg->registerDisplay<CButGraspingDisplay> ("CButGraspingDisplay");
    reg->registerDisplay<CButBBEstimationDisplay> ("CButBBEstimationDisplay");
}
