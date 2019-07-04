/*
 * EEZ Generic Firmware
 * Copyright (C) 2016-present, Envox d.o.o.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>

#include <eez/gui/page.h>

#include <eez/util.h>

#include <eez/gui/draw.h>
#include <eez/gui/gui.h>
#include <eez/gui/app_context.h>
#include <eez/modules/mcu/display.h>

using namespace eez::mcu;

namespace eez {
namespace gui {

////////////////////////////////////////////////////////////////////////////////

void Page::pageWillAppear() {
}

bool Page::onEncoder(int counter) {
    return false;
}

bool Page::onEncoderClicked() {
    return false;
}

int Page::getDirty() {
    return 0;
}

////////////////////////////////////////////////////////////////////////////////

void SetPage::edit() {
}

void SetPage::onSetValue(float value) {
    popPage();
    SetPage *page = (SetPage *)getActivePage();
    page->setValue(value);
}

void SetPage::setValue(float value) {
}

void SetPage::discard() {
    popPage();
}

////////////////////////////////////////////////////////////////////////////////

InfoPage::InfoPage(const char *message_, void (*callback_)()) : message(message_) {
    g_appContext->m_dialogYesCallback = callback_;
}

void InfoPage::refresh() {
    DECL_STYLE(style, STYLE_ID_INFO_ALERT);

    font::Font font = styleGetFont(style);

    int textWidth = display::measureStr(message, -1, font, 0);
    int textHeight = font.getHeight();

    width = style->border_size_left + style->padding_left + 
        textWidth + 
        style->padding_right + style->border_size_right;

    height = style->border_size_top + style->padding_top + 
        textHeight + 
        style->padding_bottom + style->border_size_bottom;

	x = g_appContext->x + (g_appContext->width - width) / 2;
	y = g_appContext->y + (g_appContext->height - height) / 2;

    int x1 = x;
    int y1 = y;
    int x2 = x + width - 1;
    int y2 = y + height - 1;

    int borderRadius = style->border_radius;
    if (style->border_size_top > 0 || style->border_size_right > 0 || style->border_size_bottom > 0 || style->border_size_left > 0) {
        display::setColor(style->border_color);
        if ((style->border_size_top == 1 && style->border_size_right == 1 && style->border_size_bottom == 1 && style->border_size_left == 1) && borderRadius == 0) {
            display::drawRect(x, y, x2, y2);
        } else {
            display::fillRect(x1, y1, x2, y2, style->border_radius);
			borderRadius = MAX(borderRadius - MAX(style->border_size_top, MAX(style->border_size_right, MAX(style->border_size_bottom, style->border_size_left))), 0);
        }
        x1 += style->border_size_left;
        y1 += style->border_size_top;
        x2 -= style->border_size_right;
        y2 -= style->border_size_bottom;
    }

    // draw background
    display::setColor(style->background_color);
    display::fillRect(x1, y1, x2, y2, borderRadius);

    // draw text message
    display::setColor(style->color);
    display::drawStr(message, -1, x1 + style->padding_left, y1 + style->padding_top, x1, y1, x2, y2, font);
}

bool InfoPage::updatePage() {
    return false;
}

WidgetCursor InfoPage::findWidget(int x, int y) {
	widget.action = ACTION_ID_INTERNAL_DIALOG_YES;
	return WidgetCursor(nullptr, 0, &widget, x, y, -1, 0, 0);
}

////////////////////////////////////////////////////////////////////////////////

SelectFromEnumPage::SelectFromEnumPage(const data::EnumItem *enumDefinition_, uint8_t currentValue_,
                                       bool (*disabledCallback_)(uint8_t value),
                                       void (*onSet_)(uint8_t))
    : enumDefinition(enumDefinition_), enumDefinitionFunc(NULL),
	  currentValue(currentValue_), disabledCallback(disabledCallback_), onSet(onSet_),
	  widgetCursorAtTouchDown(getFoundWidgetAtDown())
{
}

SelectFromEnumPage::SelectFromEnumPage(void (*enumDefinitionFunc_)(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value), 
                                       uint8_t currentValue_, bool (*disabledCallback_)(uint8_t value), void (*onSet_)(uint8_t))
    : enumDefinition(NULL), enumDefinitionFunc(enumDefinitionFunc_),
	  currentValue(currentValue_), disabledCallback(disabledCallback_), onSet(onSet_),
	  widgetCursorAtTouchDown(getFoundWidgetAtDown())
{
}

uint8_t SelectFromEnumPage::getValue(int i) {
    if (enumDefinitionFunc) {
        data::Value value;
        data::Cursor cursor(i);
        enumDefinitionFunc(data::DATA_OPERATION_GET_VALUE, cursor, value);
        return value.getUInt8();
    }
    
    return enumDefinition[i].value;
}

const char *SelectFromEnumPage::getLabel(int i) {
    if (enumDefinitionFunc) {
        data::Value value;
        data::Cursor cursor(i);
        enumDefinitionFunc(data::DATA_OPERATION_GET_LABEL, cursor, value);
        return value.getString();
    }

    return enumDefinition[i].menuLabel;
}

bool SelectFromEnumPage::isDisabled(int i) {
    return disabledCallback && disabledCallback(getValue(i));
}

void SelectFromEnumPage::findPagePosition() {
	x = widgetCursorAtTouchDown.x;
	y = widgetCursorAtTouchDown.y + widgetCursorAtTouchDown.widget->h;
	if (x + width <= display::getDisplayWidth() && y + height <= display::getDisplayHeight()) {
		return;
	}

	x = widgetCursorAtTouchDown.x + widgetCursorAtTouchDown.widget->w - width;
	y = widgetCursorAtTouchDown.y + widgetCursorAtTouchDown.widget->h;
	if (x + width <= display::getDisplayWidth() && y + height <= display::getDisplayHeight()) {
		return;
	}

	x = widgetCursorAtTouchDown.x;
	y = widgetCursorAtTouchDown.y - height;
	if (x + width <= display::getDisplayWidth() && y + height <= display::getDisplayHeight()) {
		return;
	}

	x = widgetCursorAtTouchDown.x + widgetCursorAtTouchDown.widget->w - width;
	y = widgetCursorAtTouchDown.y - height;
	if (x + width <= display::getDisplayWidth() && y + height <= display::getDisplayHeight()) {
		return;
	}

	x = (display::getDisplayWidth() - width) / 2;
	y = (display::getDisplayHeight() - height) / 2;
}

void SelectFromEnumPage::refresh() {
    DECL_STYLE(containerStyle, STYLE_ID_SELECT_ENUM_ITEM_POPUP_CONTAINER);
    DECL_STYLE(itemStyle, STYLE_ID_SELECT_ENUM_ITEM_POPUP_ITEM);
    DECL_STYLE(disabledItemStyle, STYLE_ID_SELECT_ENUM_ITEM_POPUP_DISABLED_ITEM);

    font::Font font = styleGetFont(itemStyle);

    // calculate geometry
    itemHeight = itemStyle->padding_left + font.getHeight() + itemStyle->padding_right;
    itemWidth = 0;

    int i;

    char text[64];

    for (i = 0; getLabel(i); ++i) {
        getItemLabel(i, text, sizeof(text));
        int width = display::measureStr(text, -1, font);
        if (width > itemWidth) {
            itemWidth = width;
        }
    }

    itemWidth = itemStyle->padding_left + itemWidth + itemStyle->padding_right;

    numItems = i;

    width = containerStyle->padding_left + itemWidth + containerStyle->padding_right;
    if (width > display::getDisplayWidth()) {
        width = display::getDisplayWidth();
    }

    height =
        containerStyle->padding_top + numItems * itemHeight + containerStyle->padding_bottom;
    if (height > display::getDisplayHeight()) {
        height = display::getDisplayHeight();
    }

	findPagePosition();

    // draw background
    display::setColor(containerStyle->background_color);
    display::fillRect(x, y, x + width - 1, y + height - 1);

    // draw labels
    for (i = 0; getLabel(i); ++i) {
        int xItem, yItem;
        getItemPosition(i, xItem, yItem);

        getItemLabel(i, text, sizeof(text));
        drawText(text, -1, xItem, yItem, itemWidth, itemHeight,
                 isDisabled(i) ? disabledItemStyle : itemStyle, nullptr, false, false, false,
                 nullptr);
    }
}

bool SelectFromEnumPage::updatePage() {
    return false;
}

WidgetCursor SelectFromEnumPage::findWidget(int x, int y) {
    int i;

    for (i = 0; getLabel(i); ++i) {
        int xItem, yItem;
        getItemPosition(i, xItem, yItem);
        if (!isDisabled(i)) {
        	if (x >= xItem && x < xItem + itemWidth && y >= yItem && y < yItem + itemHeight) {
        		widget.action = ACTION_ID_INTERNAL_SELECT_ENUM_ITEM;
        		widget.data = (uint16_t)i;
        		// TODO can't leave nullptr here
        		return WidgetCursor(nullptr, 0, &widget, x, y, -1, 0, 0);
        	}
        }
    }

    return WidgetCursor();
}

void SelectFromEnumPage::selectEnumItem() {
    int itemIndex = getFoundWidgetAtDown().widget->data;
    onSet(getValue(itemIndex));
}

void SelectFromEnumPage::getItemPosition(int itemIndex, int &xItem, int &yItem) {
    DECL_STYLE(containerStyle, STYLE_ID_SELECT_ENUM_ITEM_POPUP_CONTAINER);

    xItem = x + containerStyle->padding_left;
    yItem = y + containerStyle->padding_top + itemIndex * itemHeight;
}

void SelectFromEnumPage::getItemLabel(int itemIndex, char *text, int count) {
    if (getValue(itemIndex) == currentValue) {
        text[0] = (char)142;
    } else {
        text[0] = (char)141;
    }

    text[1] = ' ';

    strncpy(text + 2, getLabel(itemIndex), count - 3);

    text[count - 1] = 0;
}

} // namespace gui
} // namespace eez
