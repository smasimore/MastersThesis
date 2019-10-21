#include <set>

#include "StateVector.hpp"

/*************************** PUBLIC FUNCTIONS *********************************/

Error_t StateVector::createNew (StateVector::StateVectorConfig_t& config,
                                std::shared_ptr<StateVector>& pStateVectorRet)
{
    Error_t ret = E_SUCCESS;

    // Verify config.
    ret = StateVector::verifyConfig (config);
    if (ret != E_SUCCESS)
    {
        return ret;
    }

    // Create State Vector.
    pStateVectorRet.reset (new StateVector (config));

    return E_SUCCESS;
}

/**************************** PRIVATE FUNCTIONS *******************************/

StateVector::StateVector (StateVector::StateVectorConfig_t& config) 
{
    // TODO(smasimore): Implement.
}

Error_t StateVector::verifyConfig (StateVector::StateVectorConfig_t& config)
{
    // Verify config not empty.
    if (config.size () == 0)
    {
        return E_EMPTY_CONFIG;
    }

    // Verify elements list is not empty, elements are unique, and enums are 
    // valid.
    std::set<StateVectorRegion_t>  regSet;
    std::set<StateVectorElement_t> elemSet;
    for (uint32_t i_region = 0; i_region < config.size (); i_region++)
    {
        StateVector::RegionConfig_t               regConfig = config[i_region];
        StateVectorRegion_t                       reg       = regConfig.region;
        std::vector<StateVector::ElementConfig_t> regElems  = regConfig.elems;

        // Verify region's elems list not empty.
        if (regElems.size () == 0)
        {
            return E_EMPTY_ELEMS;
        }

        // Verify valid region enum.
        if (reg >= SV_REG_LAST)
        {
            return E_INVALID_ENUM;
        }

        // Insert into region set. If region already in set, return error.
        if ((regSet.insert (reg)).second == false)
        {
            return E_DUPLICATE_REGION;
        }

        // Loop through elements.
        for (uint32_t i_elem = 0; i_elem < regElems.size (); i_elem++)
        {
            StateVector::ElementConfig_t elemConfig = regElems[i_elem];
            StateVectorElement_t         elem       = elemConfig.elem;
            StateVectorElementType_t     elemType   = elemConfig.type;

            // Verify valid elem enum.
            if (elem >= SV_ELEM_LAST)
            {
                return E_INVALID_ENUM;
            }

            // Verify valid type enum.
            if (elemType >= SV_T_LAST)
            {
                return E_INVALID_ENUM;
            }

            // Insert into elem set.
            if ((elemSet.insert (elem)).second == false)
            {
                return E_DUPLICATE_ELEM;
            }
        }
    }

    return E_SUCCESS;
}

