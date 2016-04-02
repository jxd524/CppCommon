#include "stdafx.h"
#include "JxdPageManage.h"

CxdPageManage::CxdPageManage()
{
    m_bUnion = true;
    m_nCurPageIndex = -1;
}

CxdPageManage::~CxdPageManage()
{
    for ( TPageGrounpListIT it = m_ltPageGrounpList.begin(); it != m_ltPageGrounpList.end(); it++ )
    {
        TPageGrounp *pGounp = *it;
        delete pGounp;
    }
    m_ltPageGrounpList.clear();
}

UINT CxdPageManage::AddPage( void )
{
    TPageGrounp *pGrounp = new TPageGrounp;
    m_ltPageGrounpList.push_back( pGrounp );
    return (UINT)m_ltPageGrounpList.size() - 1;
}

bool CxdPageManage::DelPage( const UINT &AIndex )
{
    if ( AIndex < m_ltPageGrounpList.size() )
    {
        TPageGrounp *pGrounp = m_ltPageGrounpList[AIndex];
        m_ltPageGrounpList.erase( m_ltPageGrounpList.begin() + AIndex );
        delete pGrounp;
        return true;
    }
    return false;
}

bool CxdPageManage::AddComponent( const UINT &APageIndex, CxdUIComponent* ApCtrl )
{
    if ( APageIndex < m_ltPageGrounpList.size() )
    {
        TPageGrounp *pGrounp = m_ltPageGrounpList[APageIndex];
        if ( !IsExsitsComponent(pGrounp, ApCtrl) )
        {
            bool bAdd( true );
            if ( m_bUnion )
            {
                for ( TPageGrounpListIT it = m_ltPageGrounpList.begin(); it != m_ltPageGrounpList.end(); it++ )
                {
                    TPageGrounp *pItem = *it;
                    if ( pItem != pGrounp )
                    {
                        if ( IsExsitsComponent(pItem, ApCtrl) )
                        {
                            bAdd = false;
                            break;
                        }
                    }
                }
            }
            if ( bAdd )
            {
                pGrounp->push_back( ApCtrl );
                return true;
            }            
        }
    }
    return false;
}

bool CxdPageManage::IsExsitsComponent( TPageGrounp *ApPage, CxdUIComponent* ApCtrl )
{
    for ( TPageGrounpIT it = ApPage->begin(); it != ApPage->end(); it++ )
    {
        CxdUIComponent* pCtrl = *it;
        if ( pCtrl == ApCtrl )
        {
            return true;
        }
    }
    return false;
}


bool CxdPageManage::DeleteComponent( TPageGrounp *ApPage, CxdUIComponent* ApCtrl )
{
    for ( TPageGrounpIT it = ApPage->begin(); it != ApPage->end(); it++ )
    {
        CxdUIComponent* pCtrl = *it;
        if ( pCtrl == ApCtrl )
        {
            ApPage->erase( it );
            return true;
        }
    }
    return false;
}


bool CxdPageManage::DelComponent( CxdUIComponent* ApCtrl )
{
    for ( TPageGrounpListIT it = m_ltPageGrounpList.begin(); it != m_ltPageGrounpList.end(); it++ )
    {
        TPageGrounp *pItem = *it;
        if ( DeleteComponent(pItem, ApCtrl) )
        {
            return true;
        }
    }
    return false;
}

TPageGrounp* CxdPageManage::GetPageInfo( const UINT &AIndex )
{
    if ( AIndex < m_ltPageGrounpList.size() )
    {
        return m_ltPageGrounpList[AIndex];
    }
    return NULL;
}

void CxdPageManage::SetCurPageIndex( const int &AIndex )
{
    if ( AIndex == m_nCurPageIndex )
    {
//        return;
    }
    int nSize = (int)m_ltPageGrounpList.size();
    if ( m_nCurPageIndex == -1 )
    {
        for (int i = 0; i != nSize; i++)
        {
            TPageGrounp *pPage = GetPageInfo(i);
            SetPageCtrlsVisible( pPage, false );
        }
    }
    else if ( m_nCurPageIndex >= 0 && m_nCurPageIndex < nSize )
    {
        TPageGrounp *pPage = GetPageInfo(m_nCurPageIndex);
        SetPageCtrlsVisible( pPage, false );
    }
    if ( AIndex >= 0 && AIndex < nSize )
    {
        m_nCurPageIndex = AIndex;
        TPageGrounp *pPage = GetPageInfo(m_nCurPageIndex);
        SetPageCtrlsVisible( pPage, true );
    }
    else if ( -1 == AIndex )
    {
        m_nCurPageIndex = -1;
    }
}

void CxdPageManage::SetPageCtrlsVisible( TPageGrounp *ApPage, bool bVisible )
{
    if ( !Assigned(ApPage) ) return;

    for ( TPageGrounpIT it = ApPage->begin(); it != ApPage->end(); it++ )
    {
        CxdUIComponent* pCtrl = *it;
        pCtrl->SetVisible( bVisible );
        pCtrl->Invalidate();     
    }
}

void CxdPageManage::SetPageCtrlsEnable( TPageGrounp *ApPage, bool bEnable )
{
    if ( !Assigned(ApPage) ) return;

    for ( TPageGrounpIT it = ApPage->begin(); it != ApPage->end(); it++ )
    {
        CxdUIComponent* pCtrl = *it;
        pCtrl->SetEnable( bEnable );
    }
}

void CxdPageManage::SetAllPageVisible( bool aVisible )
{
    m_nCurPageIndex = -1;
    for( int i = 0; i != GetPageCount(); i++ )
    {
    	SetPageCtrlsVisible( GetPageInfo(i), aVisible );
    }
}
