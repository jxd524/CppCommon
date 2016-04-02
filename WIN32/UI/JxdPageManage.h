#pragma once;
#include <vector>
using namespace std;

#include "JxduiBasic.h"

typedef vector<CxdUIComponent*> TPageGrounp;
typedef TPageGrounp::iterator TPageGrounpIT;
typedef vector<TPageGrounp*> TPageGrounpList;
typedef TPageGrounpList::iterator TPageGrounpListIT;

class CxdPageManage
{
public:
    CxdPageManage();
    ~CxdPageManage();

    inline int GetCurPageIndex(void) { return m_nCurPageIndex; }
    inline UINT GetPageCount(void) { return (UINT)m_ltPageGrounpList.size(); }
    //��ӵ��������еĿؼ���Ψһ��, һ���ؼ��ӵ������ҳ��
    inline bool GetUnion(void) { return m_bUnion; } 
    inline void SetUnion(bool bUnion) { m_bUnion = bUnion; }

    UINT AddPage(void);
    bool DelPage(const UINT &AIndex);
    bool AddComponent(const UINT &APageIndex, CxdUIComponent* ApCtrl);
    bool DelComponent(CxdUIComponent* ApCtrl);

    void SetCurPageIndex(const int &AIndex); //���ÿɼ���
    void SetAllPageVisible(bool aVisible);

    void SetPageCtrlsVisible(TPageGrounp *ApPage, bool bVisible); 
    void SetPageCtrlsEnable(TPageGrounp *ApPage, bool bEnable);

    TPageGrounp* GetPageInfo(const UINT &AIndex); 
private:
    int m_nCurPageIndex;
    bool m_bUnion;
    TPageGrounpList m_ltPageGrounpList;
    bool IsExsitsComponent(TPageGrounp *ApPage, CxdUIComponent* ApCtrl);
    bool DeleteComponent(TPageGrounp *ApPage, CxdUIComponent* ApCtrl);
    
};