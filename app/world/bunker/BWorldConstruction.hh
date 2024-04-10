#ifndef TB_BWorldConstruction_HH
#define TB_BWorldConstruction_HH

#include "WorldConstruction.hh"

class D3DTray;

class BBWorldConstruction: public WorldConstruction {
public:
  static WorldConstruction *GetInstance();

bool Create() override;


private:
    ///
    BWorldConstruction() = default;

    ///
    ~BWorldConstruction() = default;

    /// Delete the copy and move constructors
    BWorldConstruction(const BWorldConstruction &) = delete;
    BWorldConstruction &operator=(const BWorldConstruction &) = delete;
    BWorldConstruction(BWorldConstruction &&) = delete;
    BWorldConstruction &operator=(BWorldConstruction &&) = delete;

    ///
    std::vector<D3DTray*> m_trays;

};

#endif