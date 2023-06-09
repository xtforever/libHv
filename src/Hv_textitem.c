/*
  -----------------------------------------------------------------------
  File:     Hv_textitem.c
  ==================================================================================
  
  The Hv library was developed at CEBAF under contract to the
  Department of Energy and is the property of the U.S. Government.
  
  It may be used to develop commercial software, but the Hv
  source code and/or compiled modules/libraries may not be sold.
  
  Questions or comments should be directed to heddle@cebaf.gov
  ==================================================================================
  */

/* This file contains functions dealing with text items */

#include "Hv.h"	  /* contains all necessary include files */
#include "Hv_xyplot.h"
#include "Hv_maps.h"

/* for text item editor */

static short           newtextcolor, newfillcolor;
static Widget          textcolorlabel, fillcolorlabel;

/*------ local prototypes ------*/

static void  Hv_FixTextItemRegion(Hv_Item Item);

static void Hv_UpdateTextItem(Hv_Item  Item);

static void Hv_EditTextColor (Widget  w);

static void Hv_TextItemSingleClick(Hv_Event hvevent);

short desiredX = -32600;
short desiredY = -32600;

/* -------- Hv_SizeTextItem ----------------*/

void Hv_SizeTextItem(Hv_Item   Item,
		     short     x,
		     short     y)


/*  x, y are  left and top corner coords */
    
    
/* Set the area for the TEXT_ITEM to be just big enough to surround the text */
    
{
    short    extra;


    if ((Item == NULL) || (Item->str == NULL))
	return;
    
    if (Hv_CheckBit(Item->drawcontrol, Hv_DONTAUTOSIZE))
	return;
    

    if (Item->type == Hv_TEXTENTRYITEM)
	extra = Hv_TEXTENTRYEXTRA;
    else
	extra = Hv_STRINGEXTRA;


    Hv_CompoundStringArea(Item->str, x, y, Item->area, extra);
}

/*------ Hv_ConcatTextToTextItem --------*/

void Hv_ConcatTextToTextItem(Hv_Item   Item,
			     char     *concattext)
    
/****************************************
  Add the text to the text item and see if
  it needs to be scrolled.
  ****************************************/
    
{
    char   *text, *newtext;
    int    extra, len1, len2;
    short  room;
    short  w, h;
    
    if ((Item == NULL) || (Item->str == NULL)
	|| (concattext == NULL))
	return;
    
    text = Hv_GetItemText(Item);
    
/* next check fixes bug found by James Muguira */
    
    if (text == NULL) {
	Hv_InitCharStr(&text, concattext);
	return;
    }
    
    len1 = strlen(text);
    len2 = strlen(concattext);
    
    newtext = (char *)Hv_Malloc(len1+len2+1);
    strcpy(newtext, text);
    strcat(newtext, concattext);
    Hv_SetItemText(Item, newtext);
    
/* see if offset is required */
    
    Hv_CompoundStringDimension(Item->str, &w, &h);
    extra = 2 + Item->scheme->framewidth + Item->scheme->borderwidth;
    extra *= 2;
    room = Item->area->width - extra;
    
    Item->str->offset = Hv_sMax(0, w - room);
    
    Hv_UpdateTextItem(Item);
    
}


/*--------- Hv_ChangeDanglingItem -------------*/

void   Hv_CheckDanglingTextItem(void)
    
{
    if ((Hv_activeTextItem != NULL) && (Hv_DanglingTextItemCallback != NULL))
	Hv_DanglingTextItemCallback();
}

/*--------- Hv_ChangeTextItemText -------------*/

void  Hv_ChangeTextItemText(Hv_Item  Item,
			    char    *str,
			    Boolean  redraw)
    
{
    Hv_Region  rgn1 = NULL;
    Hv_Region  rgn2 = NULL;
    Hv_Region  rgn  = NULL;
    short      x, y, extra;
    

    if ((Item == NULL) || (Item->type != Hv_TEXTITEM) || (Item->str == NULL))
	return;

    if (Item->type == Hv_TEXTENTRYITEM)
	extra = Hv_TEXTENTRYEXTRA;
    else
	extra = Hv_STRINGEXTRA;

    
/* first get the desired x & y from the "old" area */
    
    Hv_CompoundStringXYFromArea(Item->str, &x, &y, Item->area, extra);

/* now change it */

    if (redraw)
	rgn1 = Hv_CopyItemRegion(Item, True);

    Hv_SetItemText(Item, str);
    Hv_SizeTextItem(Item, x, y);
    
    if (redraw) { /* item may have changed size ! */
	rgn2 = Hv_CopyItemRegion(Item, True);
	rgn = Hv_UnionRegion(rgn1, rgn2);
	Hv_DrawView(Item->view, rgn);
    }
    

    Hv_DestroyRegion(rgn1);
    Hv_DestroyRegion(rgn2);
    Hv_DestroyRegion(rgn);
}


/*-------- Hv_EditTextItem -------*/

void   Hv_EditTextItem(Hv_Event hvevent)
    
/* item edit for Hv_TEXTITEMs  */
    
{
    Hv_Item              Item = hvevent->item;
    static Widget        dialog = NULL;
    static Widget        strtext;
    static Widget        frame;
    Widget               dummy, rowcol, rc, rc1, rc2, rca;
    char                 *tempstr;
    int                  answer;
    static Widget        horizontal;
    short                extra;
    
/* widgets for the newfangled "suite" of font controls */
    
    static Hv_Widget       family, size, style;
    static Hv_WidgetList   family_btns, size_btns, style_btns;
    
    
    if ((Item == NULL) || (Item->type != Hv_TEXTITEM) || (Item->str == NULL))
	return;
    
    if (!dialog) {
	Hv_VaCreateDialog(&dialog, Hv_TITLE, " Text Editor ", NULL);
	
	rowcol = Hv_DialogColumn(dialog, 8);
	
/* strtext will be used for the actual string */
	
	strtext = Hv_StandardTextField(rowcol);
	
	rc = Hv_DialogRow(rowcol, 8);
	rc1 = Hv_DialogColumn(rc, 2);
	rc2 = Hv_DialogColumn(rc, 2);
	
	rca = Hv_DialogTable(rc1, 2, 3);
	textcolorlabel = Hv_SimpleColorLabel(rca, " text ", Hv_EditTextColor);
	fillcolorlabel = Hv_SimpleColorLabel(rca, " fill ", Hv_EditTextColor);
	
/* frame the str? */
	
	frame = Hv_SimpleToggleButton(rc2, "3D frame");
	horizontal = Hv_SimpleToggleButton(rc2, "Horizontal");
	
/* font stuff */
	
	Hv_FontDialogSuite(rowcol, "font",
			   &family, &size, &style,
			   &family_btns, &size_btns, &style_btns);
	
	dummy = Hv_SimpleDialogSeparator(rowcol);
	
	
/* instructions */
	
	dummy = Hv_StandardLabel(rowcol, "Special control sequences", -1);
	
	rc = Hv_DialogTable(rowcol, 9, 6);
	dummy = Hv_StandardLabel(rc, "\\\\ back slash      ", 0);
	dummy = Hv_StandardLabel(rc, "\\n new line     ", 0);
	dummy = Hv_StandardLabel(rc, "\\b bold face    ", 0);
	dummy = Hv_StandardLabel(rc, "\\p plain face   ", 0);
	dummy = Hv_StandardLabel(rc, "\\g greek        ", 0);
	dummy = Hv_StandardLabel(rc, "\\r roman        ", 0);
	dummy = Hv_StandardLabel(rc, "\\+ bigger font  ", 0);
	dummy = Hv_StandardLabel(rc, "\\- smaller font ", 0);
	dummy = Hv_StandardLabel(rc, "\\^ superscript  ", 0);
	dummy = Hv_StandardLabel(rc, "\\_ subscript    ", 0);
	dummy = Hv_StandardLabel(rc, "\\s small space  ", 0);
	dummy = Hv_StandardLabel(rc, "\\S big space    ", 0);
	dummy = Hv_StandardLabel(rc, "\\d default      ", 0);
	dummy = Hv_StandardLabel(rc, "\\c center       ", 0);
	dummy = Hv_StandardLabel(rc, "\\l left align   ", 0);
	
	rc = Hv_StandardActionButtons(rowcol, 30, Hv_ALLACTIONS);
    }

/* get the old x & y */

    if (Item->type == Hv_TEXTENTRYITEM)
	extra = Hv_TEXTENTRYEXTRA;
    else
	extra = Hv_STRINGEXTRA;


    Hv_CompoundStringXYFromArea(Item->str, &desiredX, &desiredY, Item->area, extra);
    
/* the dialog has been created */
    
    Hv_SetToggleButton(horizontal, Item->str->horizontal);
    
/* set the current font in the option menu */
    
    Hv_FontToFontParameters(Item->str->font, &Hv_hotFontFamily, &Hv_hotFontSize, &Hv_hotFontStyle);
    Hv_SetOptionMenu(family, family_btns, (int)Hv_hotFontFamily);
    Hv_SetOptionMenu(size,   size_btns,   (int)Hv_hotFontSize);
    Hv_SetOptionMenu(style,  style_btns,  (int)Hv_hotFontStyle);
    
    Hv_SetDeleteItem(dialog, Item);
    
    newtextcolor = Item->str->strcol;
    newfillcolor = Item->str->fillcol;
    
    Hv_ChangeLabelColors(textcolorlabel, -1, Item->str->strcol);
    Hv_ChangeLabelColors(fillcolorlabel, -1, Item->str->fillcol);
    
    Hv_SetString(strtext, Hv_GetItemText(Item));
    Hv_SetToggleButton(frame, Hv_CheckBit(Item->drawcontrol, Hv_FRAMEAREA));
    
    while ((answer = Hv_DoDialog(dialog, NULL)) > 0) {
	tempstr = Hv_GetString(strtext);
	Hv_SetItemText(Item, tempstr);
	Hv_Free(tempstr);
	
	Item->str->horizontal = Hv_GetToggleButton(horizontal);
	
	Item->str->font = Hv_FontParametersToFont(Hv_hotFontFamily, Hv_hotFontSize, Hv_hotFontStyle);
	Item->str->strcol  = newtextcolor;
	Item->str->fillcol = newfillcolor;
	
	if (Hv_GetToggleButton(frame))
	    Hv_SetBit(&(Item->drawcontrol), Hv_FRAMEAREA);
	else
	    Hv_ClearBit(&(Item->drawcontrol), Hv_FRAMEAREA);
	
/* now handle the redraw -- including the fact the the size of the item probably changed */
	
	Hv_StandardRedoItem(Item);
	
	if (answer != Hv_APPLY)
	    break;
    }

    desiredX = -32600;
    
}

/* ------- Hv_TextButtonCallback -------*/

void Hv_TextButtonCallback(Hv_Event hvevent)
    
{
    Hv_View              View = hvevent->view;
    static Hv_View       activeView = NULL;
    static Hv_Item       activeItem = NULL;
    Hv_Item              newItem;
    static Cursor        tempcursor;
    Hv_Point             pp;
    short                w, h;

    Hv_Item              parent = NULL;
    
    if (activeView == NULL) {
	Hv_StandardFirstEntry(hvevent, Hv_TextButtonCallback, &activeView, &activeItem, &tempcursor);
	return;
    }
    
    if (View != activeView) {
	Hv_Warning("Text annotator active on another view!");
	return;
    }
    
    Hv_ResetActiveView(hvevent, &activeView, &activeItem, tempcursor);
    
/* see if click occured inside hotrect */
    
    Hv_SetPoint(&pp, hvevent->where.x, hvevent->where.y); 
    if (Hv_PointInRect(pp, View->hotrect)) {



	
/* for certain items, we assume the desire is to contain the
   new item */
	
	parent = Hv_PointInPlotItem(View, pp);
	
	newItem = Hv_VaCreateItem(View,
				  Hv_PARENT,       parent,
				  Hv_TAG,          Hv_DRAWINGTOOLTAG,
				  Hv_AFTERDRAG,    Hv_TextItemAfterDrag,
				  Hv_AFTEROFFSET,  Hv_TextItemAfterOffset,
				  Hv_FIXREGION,    Hv_FixTextItemRegion,
				  Hv_TYPE,         Hv_TEXTITEM,
				  Hv_DOMAIN,       Hv_INSIDEHOTRECT,
				  Hv_DRAWCONTROL,  Hv_DRAGGABLE,
				  Hv_LEFT,         pp.x,
				  Hv_TOP,          pp.y,
				  Hv_FONT,         Hv_hotFont,
				  Hv_TEXT,         "",
				  NULL);
	hvevent->item = newItem;
	
	
/* now set the properties based on the view's defaults */
	
	if (Hv_usedNewDrawTools) {
	    newItem->str->font = View->hotfont;
	    
	    Hv_usedNewDrawTools = False;
	}
	
	if (View->tag == Hv_XYPLOTVIEW)
	    Hv_SetItemDrawControlBit(newItem, Hv_INBACKGROUND);
	
	newItem->doubleclick(hvevent);

	if (Hv_IsMapView(View) && (newItem->str != NULL)) {
	    Hv_CompoundStringDimension(newItem->str, &w, &h);
	    if (newItem->base == NULL)
		newItem->base = (Hv_FPoint *)Hv_Malloc(sizeof(Hv_FPoint));
	    Hv_LocalToLatLong(View,
			      pp.x, pp.y+h,
			      &(newItem->base->h), &(newItem->base->v));
	}



/*===
 * see if the user wants to be notified
 *===*/

	if (Hv_CreateDrawingItemCB != NULL)
	  Hv_CreateDrawingItemCB(newItem);

    } 
}


/* -------- Hv_DrawTextItem --------------- */

void Hv_DrawTextItem(Hv_Item    Item,
		     Hv_Region   region)
    
/*draws the specified Item Hv_TEXTITEM*/
    
{
    short   	   xmw, xmh;		        /* String width & height in pixels */
    Hv_Pixmap	   pmap1, pmap2;		/* off screen pixmaps required for rotating text */
    int		   i,j;			        /* loopers */
    short		   desty;		        /* destination vert pixel */
    XImage           *image1, *image2;            /* images for storing text and rotated text */
    Hv_Region        temp, temp2;
    unsigned long    oldfg;
    Hv_Rect          *area;
    Hv_Rect          cliparea;
    Hv_ColorScheme   *scheme;
    Hv_String        *str;
    Window           twin;
    Hv_Rect          parea, psrect;
    Hv_Region        areareg, totreg;
    short            x, y, extra;
    
    scheme = Item->scheme;
    str = Item->str;
    
/* set up the motif string and get some characteristics */
    
    area = Item->area;



    if (Item->type == Hv_TEXTENTRYITEM)
	extra = Hv_TEXTENTRYEXTRA;
    else
	extra = Hv_STRINGEXTRA;

/* fix the area for a map based text item */

    if (Hv_IsMapView(Item->view) && 
	(Item->type == Hv_TEXTITEM) &&
	(Item->domain == Hv_INSIDEHOTRECT)) {

	if (Item->base == NULL) {


	    Item->base = (Hv_FPoint *)Hv_Malloc(sizeof(Hv_FPoint));
	    Hv_LocalToLatLong(Item->view,
			      Item->area->left, Item->area->bottom,
			      &(Item->base->h), &(Item->base->v));
	}

	Item->fixregion(Item);
    }


    Hv_CompoundStringXYFromArea(Item->str, &x, &y, area, extra);

    Hv_CopyRect(&cliparea, area);
    
    if (Item->domain == Hv_INSIDEHOTRECT)
	Hv_LocalRectToWorldRect(Item->view, area, Item->worldarea);
    
    if ((Item->str->horizontal) || (Hv_inPostscriptMode)) {
	
	areareg = Hv_RectRegion(&cliparea);
	totreg = Hv_IntersectRegion(region, areareg);
	
	if (Hv_inPostscriptMode) {
	    Hv_LocalRectToPSLocalRect(Item->area, &psrect);
	    Hv_PSClipRect(&psrect);
	}		
	
	
	Hv_SetClippingRegion(totreg);
	
	if (str->fillcol >= 0) {
	    
/* hack for plot labels in ps */
	    
	    if (!Hv_inPostscriptMode ||
		((Item->tag != Hv_XYTITLETAG) &&
		 (Item->tag != Hv_XLABELTAG) &&
		 (Item->tag != Hv_YLABELTAG)))
		
		Hv_FillRect(area, str->fillcol);
	    
	}
	else {
	    if (Item->domain != Hv_INSIDEHOTRECT)
		Hv_FillRect(area, scheme->fillcolor);
	}
	
	
	oldfg = Hv_SwapForeground(str->strcol);
	
	Hv_DrawCompoundString(x, y, str);
	
/* add a 3D frame? */
	
	Hv_DestroyRegion(areareg);
	Hv_DestroyRegion(totreg);
	
	Hv_RestoreClipRegion(region);
	
	if (Hv_CheckBit(Item->drawcontrol, Hv_FRAMEAREA))
	    Hv_Simple3DRect(area, scheme->topcolor < scheme->bottomcolor, -1);
	Hv_SetForeground(oldfg);
    } /* end horizontal OR postscript */
    
    else {   /* vertical AND not postscript */
	
/* if it is a vertical string set some size info and
   rotate the string from horiz to vert
   note: the "extra" IS part of the pixmaps */
	
	xmw = area->height;
	xmh = area->width;
	
	pmap1 = Hv_CreatePixmap((int)xmw, (int)xmh);  /* horizontal */
	pmap2 = Hv_CreatePixmap((int)xmh, (int)xmw);  /* vertical */
	
/* Draw string onto offscreen horizontal pixmap */
	
	twin = Hv_mainWindow;
	Hv_mainWindow = pmap1;
	
	Hv_SetRect(&parea, 0, 0, (int)xmw+1, (int)xmh+1);
	
/* note: I will not be here if in postsscript mode */
	
	Hv_ClipRect(&parea);
	
	if (str->fillcol >= 0)
	    Hv_FillRect(&parea, str->fillcol);
	else {
	    if (Item->domain != Hv_INSIDEHOTRECT)
		Hv_FillRect(&parea, scheme->fillcolor);
	    else { /* inside hotrect */
		if ((Item->view->hotrectborderitem) && (Item->view->hotrectborderitem->scheme))
		    Hv_FillRect(&parea, Item->view->hotrectborderitem->scheme->fillcolor);
	    }
	}

	Hv_CompoundStringXYFromArea(Item->str, &x, &y, Item->area, extra);
	x = x - Item->area->left;
	y = Item->area->bottom - y;
	
	Hv_DrawCompoundString(y, x, str);
	Hv_mainWindow = twin;
	
/* NEW ALGORITHM:`<
   pmap1 -> image1;
   pmap2 -> imag2;
   image1 -> image2;
   image2 -> screen
   
   Manipulate the images, which are stored in the client */
	
	image1 = XGetImage(Hv_display,
			   (Pixmap)pmap1,
			   0,
			   0,
			   (int)xmw,
			   (int)xmh,
			   AllPlanes,
			   ZPixmap);
	
	image2 = XGetImage(Hv_display,
			   (Pixmap)pmap2,
			   0,
			   0,
			   (int)xmh,
			   (int)xmw,
			   AllPlanes,
			   ZPixmap);
	
/* now rotate text pixels from image1 onto image 2 */
	
	for (i = 1; i < (int)xmw; i++) {
	    desty = xmw-i;
	    for (j = 1; j < (int)xmh; j++)
		XPutPixel(image2, j, desty, XGetPixel(image1, i, j));
	}
	
	
/* Now that image2 contains the rotated text, put it onto the screen */
	
	temp2 = Hv_RectRegion(area);       /* set a region to the rect */
	Hv_ShrinkRegion(temp2, -1, -1);
	temp = Hv_IntersectRegion(region, temp2);
	Hv_SetClippingRegion(temp);
	Hv_DestroyRegion(temp);
	Hv_DestroyRegion(temp2);
	
	if (str->fillcol >= 0)
	    Hv_FillRect(area, str->fillcol);
	else {
	    if (Item->domain != Hv_INSIDEHOTRECT)
		Hv_FillRect(area, scheme->fillcolor);
	}
	
	XPutImage(Hv_display, Hv_mainWindow, Hv_gc, image2, 1, 1,
		  area->left, area->top, xmh, xmw); 
	XFlush(Hv_display);
	
/* add a 3D frame? */
	
	if (Hv_CheckBit(Item->drawcontrol, Hv_FRAMEAREA))
	    Hv_Simple3DRect(area, scheme->topcolor < scheme->bottomcolor, -1);
	
/* free memory used by images and pixmaps */
	
	Hv_FreePixmap(pmap2);
	XDestroyImage(image2);
	XDestroyImage(image1);
	Hv_FreePixmap(pmap1);
    }  /* end vertical AND not postscript*/
}


/*------- Hv_DrawTextButton -------*/

void Hv_DrawTextButton(Hv_Item    Item,
		       Hv_Region   region)
    
/* draws the icon for text annotation buttons */
    
{
    Hv_Rect          area;
    short            xmw, xmh;		        /* String width  in pixels */
    short            xc, yoff;
    Hv_String        *str;
    
    Hv_ButtonDrawingArea(Item, &area);
    Hv_FillRect(&area, Item->scheme->fillcolor);
    
    str = Hv_CreateString("A");
    str->font = Hv_timesBold20;
    
    if (Item->drawinterval != NULL)
	str->text[0] = 'A' + (Item->drawinterval->timedredrawcount % 26);
    else 
	Hv_SetItemTimedRedraw(Item, 60000);
    
/* set up the motif string and get some characteristics */
    
    Hv_CompoundStringDimension(str, &xmw, &xmh);
    
    xc = (area.left + area.right - (short)xmw)/2 - 1;
    yoff = 1 + ((area.height - (short)xmh)/2);
    
    Hv_DrawCompoundString(xc + 2, area.top + yoff, str);
    Hv_DestroyString(str);
}


/*------- Hv_TextItemInitialize -------*/

void Hv_TextItemInitialize(Hv_Item      Item,
			   Hv_AttributeArray attributes)
    
{
    Boolean              inbox;
    short                boxfill = -1;
    Hv_Item              parent;
    
/* if in a box, use a different fillcolor */
    
    parent = Item->parent;
    inbox = ((parent != NULL) && ((parent->type == Hv_BOXITEM)
				  || (parent->type == Hv_RADIOCONTAINERITEM)));
    if (inbox)
	boxfill = parent->scheme->fillcolor;
    
    Item->standarddraw = Hv_DrawTextItem;
    Item->str = Hv_CreateString(NULL);
    Hv_DefaultSchemeIn(&(Item->scheme));
    Item->scheme->framewidth  = 2;        /* in this case it is the width of the 3d frame */
    Item->scheme->borderwidth = 2;        /* in this case it is the spacer between text and frame */
    
/* box has private fillcolor, hot rect has lighter background */
    
    if (inbox)
	Item->scheme->fillcolor = boxfill;
    else if (Item->domain == Hv_INSIDEHOTRECT)
	Item->scheme->fillcolor -= 1;
    
/* for hotrect text items, use a special single click callback that
   will allow me to duplicate */
    
    if ((Item->domain == Hv_INSIDEHOTRECT) && (Item->singleclick == NULL))
	Item->singleclick = Hv_TextItemSingleClick;
    
    
    if ((Item->domain == Hv_INSIDEHOTRECT) && (Item->doubleclick == NULL))
	Item->doubleclick = Hv_EditTextItem;  /* default doubleclick is the text item  editor */


    if (Item->domain == Hv_INSIDEHOTRECT) {
	Item->afteroffset = (Hv_FunctionPtr)Hv_TextItemAfterOffset;
	Item->afterdrag = (Hv_FunctionPtr)Hv_TextItemAfterDrag;
	Item->fixregion = (Hv_FunctionPtr)Hv_FixTextItemRegion;
    }

}

#define DEFCOPYOFFSET 10

/*-------- Hv_TextItemSingleClick --------------*/


static void Hv_TextItemSingleClick(Hv_Event hvevent)
    
/************************************************
  Mostly this just calls the default single
  click CM, UNLESS the user has pressed SHIFT
  AND andother mod key, in which case this
  duplicates the text item.
  ************************************************/
    
{
    Boolean        shift;
    Boolean        ctrl;
    Boolean        alt;
    Hv_Item        Item = hvevent->item;
    Hv_Item        newItem;
    short          xoffset, yoffset;
    static Hv_Item lastItem = NULL;
    static int     count = 0;
    int            xcnt;
    
    shift = Hv_CheckBit(hvevent->modifiers, Hv_SHIFT);
    ctrl  = Hv_CheckBit(hvevent->modifiers, Hv_CONTROL);
    alt   = Hv_CheckBit(hvevent->modifiers, Hv_MOD1);
    
    if (shift && (ctrl || alt)) {
	newItem = Hv_VaCreateItem(Item->view,
				  Hv_DRAWCONTROL,  Item->drawcontrol,
				  Hv_TAG,          Item->tag,
				  Hv_PARENT,       Item->parent,
				  Hv_TYPE,         Hv_TEXTITEM,
				  Hv_DOMAIN,       Hv_INSIDEHOTRECT,
				  NULL);

	newItem->afteroffset = (Hv_FunctionPtr)Hv_TextItemAfterOffset;
	newItem->afterdrag = (Hv_FunctionPtr)Hv_TextItemAfterDrag;
	newItem->fixregion = (Hv_FunctionPtr)Hv_FixTextItemRegion;
	
/* used lastItem to cause repeated duplicates to be offset by greater amounts */
	
	if (lastItem != Item)
	    count = 0;
	else
	    count++;
	
	if (count >= 200)
	    count = 0;
	
	xcnt = count;
	if (xcnt > 99)
	    xcnt -= 100;
	
	xoffset = ((xcnt % 10) + 1 )*DEFCOPYOFFSET;
	xoffset += 4*(xcnt/10)*DEFCOPYOFFSET;
	
	yoffset = ((count % 10) + 1 )*DEFCOPYOFFSET;
	
	if (count > 99)
	    yoffset = -yoffset;
	
	Hv_SetItemString(newItem, Item->str); 
	Hv_SetRect(newItem->area, Item->area->left+xoffset, Item->area->top+yoffset,
		   Item->area->width, Item->area->height);


/*===
 * see if the user wants to be notified
 *===*/

	if ((Item->tag == Hv_DRAWINGTOOLTAG) && (Hv_CreateDrawingItemCB != NULL))
	  Hv_CreateDrawingItemCB(newItem);




	Hv_DrawItem(newItem, NULL);
	lastItem = Item;
    }
    else
	Hv_DefaultSingleClickCallback(hvevent);
    
}

#undef DEFCOPYOFFSET

/*--------- Hv_UpdateTextItem --------*/

static void Hv_UpdateTextItem(Hv_Item Item)
    
{
    Hv_Region   viewreg, areareg, totreg;
    short       extra;
    Hv_Rect     area;
    
    
    if (Item == NULL)
	return;
    
    viewreg = Hv_GetViewRegion(Item->view);
    extra = Item->scheme->framewidth + Item->scheme->borderwidth;
    Hv_ShrinkRect(&area, extra, extra);
    Hv_ResizeRect(&area, -1, -1);
    Hv_CopyRect(&area, Item->area);
    areareg = Hv_RectRegion(&area);
    totreg = Hv_IntersectRegion(viewreg, areareg);
    Hv_DrawItem(Item, totreg);
    Hv_DestroyRegion(viewreg);
    Hv_DestroyRegion(areareg);
    Hv_DestroyRegion(totreg);
}


/* ------ Hv_EditTextColor --- */

static void Hv_EditTextColor (Widget w)
    
{
    if (w == textcolorlabel)
	Hv_ModifyColor(w, &newtextcolor, "Edit Text Color", False);
    else if (w == fillcolorlabel)
	Hv_ModifyColor(w, &newfillcolor, "Edit Fill Color", True);
}


/* -------- Hv_TextItemAfterOffset -----*/

void Hv_TextItemAfterOffset(Hv_Item  Item,
			    short   dh,
			    short   dv)
{
  short  x, y;

  if (Item == NULL)
    return;

  if ((dh == 0) && (dv == 0))
    return;

  if (!Hv_IsMapView(Item->view) || (Item->base == NULL))
    return;

  if (!Hv_offsetDueToDrag)
      return;

    if (Hv_IsMapView(Item->view) && (Item->base != NULL) && (Item->domain == Hv_INSIDEHOTRECT)) {
	Hv_LatLongToLocal(Item->view,
			  &x, &y,
			  Item->base->h, Item->base->v);
	x += dh;
	y += dv;

	Hv_LocalToLatLong(Item->view,
			  x, y,
			  &(Item->base->h),
			  &(Item->base->v));

    }

}

/* -------- Hv_TextItemAfterDrag -----*/

void Hv_TextItemAfterDrag(Hv_Item  Item,
			  short   dh,
			  short   dv)
{
}

/*------- Hv_FixTextItemRegion -------*/

static void  Hv_FixTextItemRegion(Hv_Item Item)

{
    Hv_View    View = Item->view;
    short      x, y;

    
    if (Hv_IsMapView(View) && 
	(Item->type == Hv_TEXTITEM) &&
	(Item->domain == Hv_INSIDEHOTRECT)) {

	if ((Item->str != NULL) && (Item->base != NULL)) {


/* set its area so that it will be redrawn if necessary */

	    Hv_LatLongToLocal(Item->view,
			      &x, &y,
			      Item->base->h, Item->base->v);
	    
	    Hv_OffsetRect(Item->area,
			  x - Item->area->left,
			  y - Item->area->bottom + Hv_STRINGEXTRA);

	    
	}
    }

    if (Item->worldarea != NULL)
	Hv_LocalRectToWorldRect(Item->view, Item->area, Item->worldarea);
}
