#include "includes.h"

void Checkbox::draw( ) {
	Rect area{ m_parent->GetElementsRect( ) };
	Point p{ area.x + m_pos.x, area.y + m_pos.y };

	// get gui color.
	Color color = colors::white;
	color.a( ) = m_parent->m_alpha;

	// render black outline on checkbox.
	render::rect( p.x, p.y, CHECKBOX_SIZE, CHECKBOX_SIZE, colors::white);

	// render checkbox title.
	if( m_use_label )
	    render::menu_shade.string( p.x + LABEL_OFFSET, p.y - 3, { 20,20,20,200 }, m_label );

	// render border.
	render::rect( p.x + 1, p.y + 1, CHECKBOX_SIZE - 2, CHECKBOX_SIZE - 2, colors::white);

	// render checked.
	if( m_checked ) {
		render::rect_filled( p.x + 1, p.y + 1, CHECKBOX_SIZE - 2, CHECKBOX_SIZE - 2, { 20,20,20,200 });
		render::rect_filled( p.x + 1, p.y + 1, CHECKBOX_SIZE - 2, CHECKBOX_SIZE - 2, {20,20,20,200});
	}

	else
		render::rect_filled( p.x + 1, p.y + 1, CHECKBOX_SIZE - 2, CHECKBOX_SIZE - 2, { 20,20,20,200 });

	//render::rect( el.x + m_pos.x, el.y + m_pos.y, m_w, m_pos.h, { 255, 0, 0 } );
}

void Checkbox::think( ) {
	// set the click area to the length of the string, so we can also press the string to toggle.
	if( m_use_label )
		m_w = LABEL_OFFSET + render::menu_shade.size( m_label ).m_width;
}

void Checkbox::click( ) {
	// toggle.
	m_checked = !m_checked;

	if( m_callback )
		m_callback( );
}