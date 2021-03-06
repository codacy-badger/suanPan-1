////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2017-2021 Theodore Chang
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
////////////////////////////////////////////////////////////////////////////////

#include "ArcLength.h"
#include <Converger/AbsIncreDisp.h>
#include <Domain/Domain.h>
#include <Domain/Factory.hpp>
#include <Domain/Node.h>
#include <Solver/Integrator/Integrator.h>
#include <Solver/Ramm.h>

ArcLength::ArcLength(const unsigned T, const unsigned NT, const unsigned DT, const double MA)
	: Step(T, 0.)
	, node(NT)
	, dof(DT)
	, maginitude(MA) {}

int ArcLength::initialize() {
	const auto& t_domain = database.lock();

	modifier = make_shared<Integrator>();

	if(nullptr != solver) {
		const auto& t_solver = *solver;
		if(typeid(t_solver) != typeid(Ramm)) solver = nullptr;
	}

	if(nullptr == tester) tester = make_shared<AbsIncreDisp>();
	if(nullptr == solver) solver = make_shared<Ramm>();

	tester->set_domain(t_domain);
	modifier->set_domain(t_domain);
	solver->set_converger(tester);
	solver->set_integrator(modifier);

	if(sparse_mat) factory->set_storage_scheme(StorageScheme::SPARSE);
	else if(symm_mat && band_mat) factory->set_storage_scheme(StorageScheme::BANDSYMM);
	else if(!symm_mat && band_mat) factory->set_storage_scheme(StorageScheme::BAND);
	else if(symm_mat && !band_mat) factory->set_storage_scheme(StorageScheme::SYMMPACK);
	else if(!symm_mat && !band_mat) factory->set_storage_scheme(StorageScheme::FULL);

	factory->set_analysis_type(AnalysisType::STATICS);
	factory->set_precision(precision);
	factory->set_tolerance(tolerance);
	factory->set_solver(SolverType::SPIKE == system_solver ? SolverType::LAPACK : system_solver); // ! Spike solver has no option to compute determinant

	if(SUANPAN_SUCCESS != t_domain->restart()) return SUANPAN_FAIL;

	factory->set_reference_size(1);
	factory->initialize_load_factor();

	get_reference_load(factory)(t_domain->get_node(node)->get_reordered_dof().at(dof - 1)) = maginitude;

	return SUANPAN_SUCCESS;
}

int ArcLength::analyze() {
	auto& S = get_solver();
	auto& G = get_integrator();

	unsigned num_iteration = 0;

	while(true) {
		if(num_iteration++ > get_max_substep()) {
			suanpan_warning("analyze() reaches maximum substep number %u.\n", get_max_substep());
			return SUANPAN_FAIL;
		}
		auto code = S->analyze();
		if(code == SUANPAN_SUCCESS) {
			G->commit_status();
			G->record();
			// if exit is returned, the analysis shall be terminated
			code = G->process_criterion();
			if(SUANPAN_SUCCESS != code) return code;
		} else if(code == SUANPAN_FAIL) G->reset_status();
		else return SUANPAN_FAIL;
	}
}
