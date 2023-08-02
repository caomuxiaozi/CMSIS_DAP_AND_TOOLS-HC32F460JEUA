#ifndef _INCLUDE_SGUI_MENU_H_
#define _INCLUDE_SGUI_MENU_H_
//=======================================================================//
//= Include files.                                                      =//
//=======================================================================//
#include "SGUI_Common.h"
#include "SGUI_Basic.h"
#include "SGUI_Text.h"
#include "SGUI_ItemsBase.h"
//=======================================================================//
//= Data type definition.                                               =//
//=======================================================================//
#ifdef __cplusplus
extern "C"{
#endif
typedef struct
{
    SGUI_RECT                   stLayout;
    SGUI_ITEMS_BASE             stItems;
    const SGUI_FONT_RES*        pstFontRes;
}SGUI_MENU;
#ifdef __cplusplus
}
#endif

//=======================================================================//
//= Public function declaration.                                        =//
//=======================================================================//
#ifdef __cplusplus
extern "C"{
#endif
void                    SGUI_Menu_Initialize(SGUI_MENU* pstObj, const SGUI_RECT* cpstLayout, const SGUI_FONT_RES* pstFontRes, SGUI_ITEMS_ITEM* pstItemsData, SGUI_INT iItemsCount);
void                    SGUI_Menu_Repaint(SGUI_SCR_DEV* pstDeviceIF, SGUI_MENU* pstObj);
void                    SGUI_Menu_Resize(SGUI_MENU* pstObj, const SGUI_RECT* pstNewLayout);
void                    SGUI_Menu_PopupSubMenu(SGUI_SCR_DEV* pstDeviceIF, SGUI_MENU* pstObj, const SGUI_RECT* cpstParentLayout);
#ifdef __cplusplus
}
#endif

//=======================================================================//
//= Macro function.                                                     =//
//=======================================================================//
#define                 SGUI_Menu_GetSelection(OBJ) \
                            (SGUI_ItemsBase_GetSelection(&(OBJ->stItems)))
#define                 SGUI_Menu_Selecte(OBJ, IDX) \
                            (SGUI_ItemsBase_Selecte(&(OBJ->stItems), IDX))

#define                 SGUI_Menu_CanScrollUp(OBJ) \
                            (SGUI_ItemsBase_CanScrollUp(&(OBJ->stItems)))
#define                 SGUI_Menu_CanScrollDown(OBJ) \
                            (SGUI_ItemsBase_CanScrollDown(&(OBJ->stItems)))

#endif // _INCLUDE_SGUI_MENU_H_
