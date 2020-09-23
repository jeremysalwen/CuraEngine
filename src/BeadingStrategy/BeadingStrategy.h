//Copyright (c) 2020 Ultimaker B.V.
//CuraEngine is released under the terms of the AGPLv3 or higher.

#ifndef BEADING_STRATEGY_H
#define BEADING_STRATEGY_H

#include <utility>

#include "../utils/IntPoint.h"
#include "../utils/logoutput.h"
#include "../utils/optional.h"  // until the move to C++17
#include "../settings/types/AngleRadians.h"

namespace cura
{

/*!
 * Mostly virtual base class template.
 * 
 * Strategy for covering a given (constant) horizontal model thickness with a number of beads.
 * 
 * The beads may have different widths.
 * 
 * TODO:
 * extend with printing order?
 */
class BeadingStrategy
{
public:
    /*!
     * The beading for a given horizontal model thickness.
     */
    struct Beading
    {
        coord_t total_thickness;
        std::vector<coord_t> bead_widths; //! The line width of each bead from the outer inset inward
        std::vector<coord_t> toolpath_locations; //! The distance of the toolpath location of each bead from the outline
        coord_t left_over; //! The distance not covered by any bead; gap area.
    };

    coord_t optimal_width_outer; //! Optimal bead width for the outermost wall(s)
    coord_t optimal_width_inner; //! Optimal bead width for the inner wall(s)
    
    std::string name;
    
    coord_t default_transition_length; //! The length of the region to smoothly transfer between bead counts

    /*!
     * The maximum angle between outline segments smaller than which we are going to add transitions
     * Equals 180 - the "limit bisector angle" from the paper
     */
    AngleRadians transitioning_angle;

    BeadingStrategy(coord_t optimal_width_outer, coord_t optimal_width_inner, coord_t default_transition_length, float transitioning_angle = M_PI / 3)
    : optimal_width_outer(optimal_width_outer)
    , optimal_width_inner(optimal_width_inner)
    , default_transition_length(default_transition_length)
    , transitioning_angle(transitioning_angle)
    {
        name = "Unknown";
    }

    virtual ~BeadingStrategy()
    {}

    /*!
     * Retrieve the bead widths with which to cover a given thickness.
     * 
     * Requirement: Given a constant \p bead_count the output of each bead width must change gradually along with the \p thickness.
     * 
     * \note The \p bead_count might be different from the \ref BeadingStrategy::optimal_bead_count
     */
    virtual Beading compute(coord_t thickness, coord_t bead_count) const = 0;

    /*!
     * The ideal thickness for a given \param bead_count
     */
    virtual coord_t getOptimalThickness(coord_t bead_count) const = 0;

    /*!
     * The model thickness at which \ref BeadingStrategy::optimal_bead_count transitions from \p lower_bead_count to \p lower_bead_count + 1
     */
    virtual coord_t getTransitionThickness(coord_t lower_bead_count) const = 0;

    /*!
     * The number of beads should we ideally usefor a given model thickness
     */
    virtual coord_t getOptimalBeadCount(coord_t thickness) const = 0;

    /*!
     * The length of the transitioning region along the marked / significant regions of the skeleton.
     * 
     * Transitions are used to smooth out the jumps in integer bead count; the jumps turn into ramps with some incline defined by their length.
     */
    virtual coord_t getTransitioningLength(coord_t lower_bead_count) const;

    /*!
     * The fraction of the transition length to put between the lower end of the transition and the point where the unsmoothed bead count jumps.
     * 
     * Transitions are used to smooth out the jumps in integer bead count; the jumps turn into ramps which could be positioned relative to the jump location.
     */
    virtual float getTransitionAnchorPos(coord_t lower_bead_count) const;

    /*!
     * Get the locations in a bead count region where \ref BeadingStrategy::compute exhibits a bend in the widths.
     * Ordered from lower thickness to higher.
     * 
     * This is used to insert extra support bones into the skeleton, so that the resulting beads in long trapezoids don't linearly change between the two ends.
     */
    virtual std::vector<coord_t> getNonlinearThicknesses(coord_t lower_bead_count) const;
    
    virtual std::string toString() const;
};

} // namespace cura
#endif // BEADING_STRATEGY_H