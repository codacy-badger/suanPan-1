/*******************************************************************************
 * Copyright (C) 2017-2021 Theodore Chang
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/
/**
 * @class RigidWallMultiplier
 * @brief A RigidWallMultiplier class.
 *
 * @author tlc
 * @date 15/07/2020
 * @version 0.1.0
 * @file RigidWallMultiplier.h
 * @addtogroup Constraint
 * @{
 */

#ifndef RIGIDWALLMULTIPLIER_H
#define RIGIDWALLMULTIPLIER_H

#include "Constraint/RigidWallPenalty.h"

class RigidWallMultiplier final : public RigidWallPenalty {
	const bool use_penalty = false;
public:
	using RigidWallPenalty::RigidWallPenalty;

	int initialize(const shared_ptr<DomainBase>&) override;

	int process(const shared_ptr<DomainBase>&) override;
};

#endif

//! @}
