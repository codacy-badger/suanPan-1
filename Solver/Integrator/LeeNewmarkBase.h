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
 * @class LeeNewmarkBase
 * @brief A LeeNewmarkBase class defines a solver using Newmark algorithm with Lee damping model.
 *
 * @author tlc
 * @date 27/12/2020
 * @version 0.1.0
 * @file LeeNewmarkBase.h
 * @addtogroup Integrator
 * @{
 */

#ifndef LEENEWMARKBASE_H
#define LEENEWMARKBASE_H

#include <Domain/Factory.hpp>
#include <Domain/MetaMat/MetaMat.hpp>
#include <Solver/Integrator/Newmark.h>

class LeeNewmarkBase : public Newmark {
protected:
	const uword n_block;

	bool first_iteration = true;

	vec current_internal, trial_internal, residual;

	unique_ptr<MetaMat<double>> stiffness = nullptr;

	shared_ptr<Factory<double>> factory = nullptr;

	[[nodiscard]] virtual uword get_total_size() const = 0;

	virtual void update_stiffness() const = 0;
	virtual void update_residual() const = 0;
public:
	explicit LeeNewmarkBase(unsigned, double, double);

	int initialize() override;

	int update_internal(const mat&) final;

	int solve(mat&, const mat&) final;
	int solve(mat&, const sp_mat&) final;
	int solve_trs(mat&, const mat&) final;
	int solve_trs(mat&, const sp_mat&) final;

	mat get_force_residual() final;
	mat get_displacement_residual() final;

	void commit_status() final;
	void clear_status() final;
	void reset_status() final;
};

#endif

//! @}
