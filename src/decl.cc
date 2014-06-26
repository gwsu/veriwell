/*****************************************************************************
 * Copyright 1994-2005, Elliot Mednick and Mark Hummel
 * This file is part of Veriwell.
 *
 * Veriwell is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Veriwell is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *****************************************************************************/

/* DECL.C - Declaration node generation and utilities */


#include "glue.h"
#include <stddef.h>
#include "vtypes.h"
#include "tree.h"
#include "decl.h"
#include "flags.h"		/* needed for "default_net_type" */
#include "scope.h"
#include "schedule.h"
#include "pass2.h"

/* Node for the integer constant 0 */
tree integer_zero_node;

/* Node for the integer constant 1 */
tree integer_one_node;

/* Node for the single-bit 0 */
tree bit_zero_node;

/* Node for the single-bit 1 */
tree bit_one_node;

/* Node to mark an error */
tree error_mark_node;

static void replace_port(tree here, tree ident, tree node)
{
    switch (TREE_CODE(TREE_PURPOSE(here))) {
    case IDENTIFIER_NODE:
	if (TREE_PURPOSE(here) == ident) {
	    TREE_PURPOSE(here) = node;
	}
	break;
    case REG_SCALAR_DECL:
    case REG_VECTOR_DECL:
    case NET_SCALAR_DECL:
    case NET_VECTOR_DECL:
	if (DECL_NAME(TREE_PURPOSE(here)) == ident) {
	    PORT_OUTPUT_ATTR(node) = PORT_OUTPUT_ATTR(TREE_PURPOSE(here));
	    PORT_INPUT_ATTR(node) = PORT_INPUT_ATTR(TREE_PURPOSE(here));
	    TREE_PURPOSE(here) = node;
	}
	break;
    case PART_REF:
	if (TREE_PURPOSE(here) == ident) {
	    PART_DECL(TREE_PURPOSE(here)) = node;
	    PORT_OUTPUT_ATTR(TREE_PURPOSE(here)) = PORT_OUTPUT_ATTR(node);
	    PORT_INPUT_ATTR(TREE_PURPOSE(here)) = PORT_INPUT_ATTR(node);
	    HIERARCHICAL_ATTR(TREE_PURPOSE(here)) = 0;	/* set in check */
	}
	break;
    case BIT_REF:
	if (TREE_PURPOSE(here) == ident) {
	    BIT_REF_DECL(TREE_PURPOSE(here)) = node;
	    PORT_OUTPUT_ATTR(node) = PORT_OUTPUT_ATTR(TREE_PURPOSE(here));
	    PORT_INPUT_ATTR(node) = PORT_INPUT_ATTR(TREE_PURPOSE(here));
	    HIERARCHICAL_ATTR(TREE_PURPOSE(here)) = 0;	/* set in check */
	}
	break;
    }
}

/* Search list of ports, find IDENT, and replace with NODE.  Ident may
   been replaced once before with a NET. */

void redirect_port(tree ident, tree node)
{
    tree t;
    tree t1;

    for (t = MODULE_PORT_LIST(current_scope); t; t = TREE_CHAIN(t)) {
	if (TREE_CODE(TREE_PURPOSE(t)) == TREE_LIST) {
	    for (t1 = TREE_PURPOSE(t); t1; t1 = TREE_CHAIN(t1)) {
		replace_port(t1, ident, node);
	    }
	} else {
	    replace_port(t, ident, node);
	}
    }
}

tree make_net_spec(enum tree_type net_type, tree range, tree delay)
{
    tree node;

    if (range == NULL) {
	node = make_node(NET_SCALAR_DECL);
    } else {
	node = make_node(NET_VECTOR_DECL);
	DECL_MSB(node) = TREE_OPERAND(range, 0);
	DECL_LSB(node) = TREE_OPERAND(range, 1);
    }
    DECL_CONTEXT(node) = current_scope;
    NET_DELAY(node) = delay;
    TREE_TYPE(node) = net_type;
    NET_SOURCE(node) = NULL_TREE;
    return node;
}

tree make_reg_spec(tree range)
{
    tree node;

    if (range == NULL) {
	node = make_node(REG_SCALAR_DECL);
    } else {
	node = make_node(REG_VECTOR_DECL);
	DECL_MSB(node) = TREE_OPERAND(range, 0);
	DECL_LSB(node) = TREE_OPERAND(range, 1);
    }
    DECL_CONTEXT(node) = current_scope;
    return node;
}

tree make_integer_spec(tree range)
{
    tree node;

    node = make_node(INTEGER_DECL);
    if (range == NULL) {
	DECL_MSB(node) = NULL_TREE;
	DECL_LSB(node) = NULL_TREE;
    } else {
	DECL_MSB(node) = TREE_OPERAND(range, 0);
	DECL_LSB(node) = TREE_OPERAND(range, 1);
    }
    DECL_CONTEXT(node) = current_scope;
    TREE_INTEGER_ATTR(node) = 1;
    return node;
}

tree make_real_spec(tree range)
{
    tree node;

    node = make_node(REAL_DECL);
    if (range == NULL) {
	DECL_MSB(node) = NULL_TREE;
	DECL_LSB(node) = NULL_TREE;
    } else {
	DECL_MSB(node) = TREE_OPERAND(range, 0);
	DECL_LSB(node) = TREE_OPERAND(range, 1);
    }
    DECL_CONTEXT(node) = current_scope;
    TREE_REAL_ATTR(node) = 1;
    return node;
}

tree make_time_spec(tree range)
{
    tree node;

    node = make_node(TIME_DECL);
    if (range == NULL) {
	DECL_MSB(node) = NULL_TREE;
	DECL_LSB(node) = NULL_TREE;
    } else {
	DECL_MSB(node) = TREE_OPERAND(range, 0);
	DECL_LSB(node) = TREE_OPERAND(range, 1);
    }
    DECL_CONTEXT(node) = current_scope;
    return node;
}

tree make_event_spec()
{
    tree node = make_node(EVENT_DECL);
    DECL_CONTEXT(node) = current_scope;
    return node;
}

tree make_param_spec(tree range)
{
    tree node = make_node(PARAM_DECL);

    TREE_CONSTANT_ATTR(node) = 1;	/* treat parameter as a constant */
    if (range == NULL) {
	DECL_MSB(node) = NULL_TREE;
	DECL_LSB(node) = NULL_TREE;
    } else {
	DECL_MSB(node) = TREE_OPERAND(range, 0);
	DECL_LSB(node) = TREE_OPERAND(range, 1);
    }
    DECL_CONTEXT(node) = current_scope;
    return node;
}

/* Take a declaration and assign a name to it and possible array bounds.
   If more than one name associated with a declarations (reg [31:0] a, b, c)
   copy the original node. */

tree make_decl(tree ident, tree spec, tree hibounds, tree lobounds)
{
    tree node = spec;
    tree decl;

    if (ident == error_mark_node) {
	return copy_node(error_mark_node);
    }

    /* 
     * if no name associated with declaration, 
     * use this node, else make a copy 
     */
    if (DECL_NAME(node) != NULL_TREE) {
	node = copy_node(node);
    }
    DECL_NAME(node) = root_port_name(ident);	/* Give it a name */

    if (TREE_CODE(node) == NET_SCALAR_DECL ||
	TREE_CODE(node) == NET_VECTOR_DECL) {
	NET_SOURCE(node) = NULL_TREE;
	/* Make sure TRI1 and TRI0 get scheduled to be eval'ed in time 0,
	 * Do this by making a dummy driver; con't assgn initialization 
	 * will know what to do. 
	 */
	if (TREE_TYPE(node) == NET_TRI1_TYPE ||
	    TREE_TYPE(node) == NET_TRI0_TYPE ||
	    TREE_TYPE(node) == NET_SUPPLY0_TYPE ||
	    TREE_TYPE(node) == NET_SUPPLY1_TYPE) {
	    add_cont_assign_list(make_net_source(node));
	}
    }

    /* If this is an array, build a new node with array bounds */
    if (hibounds) {		/* implies both hi and lo are non-null */
	node = build_array(ident, node, hibounds, lobounds);
    }

    /* If current_decl of ident is non-NULL, then it is either a port
       being declared for the first time (with INPUT, OUTPUT, INOUT),
       or it is a redeclaration of a declared port (to qualify the net type,
       the size, or to turn it into a REG). */

    /* If this is a redeclaration of a port, go back to the port list and
       make it point to the new DECL node. */

    decl = IDENT_CURRENT_DECL(ident);

    if (decl) {
	/* Case 1, we are at the INPUT, OUTPUT, or INOUT decl; 
	 * search for the port and redeclare as a net with the 
	 * proper width, etc.  
	 */
	if (TREE_CODE(root_port_decl(ident)) == IDENTIFIER_NODE) {
	    redirect_port(ident, node);
	    /* Case 2: previously declared as a port, 
	     * this is the explicit declaration (WIRE, REG, etc.) 
	     */
	} else if (TREE_CODE(decl) != BLOCK_DECL && DECL_CONTEXT(decl) == DECL_CONTEXT(node)) {	/* 2nd time decl */
	    if (REFERENCED_ATTR(decl)) {
		error("Redeclared port '%s' has already been referenced",
		      IDENT(ident), NULL_CHAR);
	    }
	    DECL_THREAD(decl) = node;
	    PORT_REDEFINED_ATTR(decl) = 1;
	    PORT_INPUT_ATTR(node) = PORT_INPUT_ATTR(decl);
	    PORT_OUTPUT_ATTR(node) = PORT_OUTPUT_ATTR(decl);
	}
    }
    set_decl(root_port_name(ident), node);
    return node;
}

tree make_param_decl(tree ident, tree spec, tree rval)
{
    tree node = spec;
    /*
     * if no name associated with declaration, 
     * use this node, else make a copy 
     */
    if (DECL_NAME(node) != NULL_TREE) {
	node = copy_node(node);
    }
    DECL_NAME(node) = ident;	/* Give it a name */
    set_decl(ident, node);
    DECL_PARAM_RVAL(node) = rval;
    DECL_PARAM_REDIRECT(node) = NULL_TREE;
    TREE_CONSTANT_ATTR(node) = 1;
    return node;
}

/* make a decl for a named BLOCK or task or function */

tree make_block_decl(tree ident, tree context, tree block)
{
    tree decl = make_node(BLOCK_DECL);

    if (ident == error_mark_node) {
	return ident;
    }
    DECL_CONTEXT(decl) = context;
    DECL_THREAD(decl) = block;
    DECL_NAME(decl) = ident;
    set_decl(ident, decl);
    TREE_CHAIN(decl) = BLOCK_DECL(context);
    BLOCK_DECL(context) = decl;
    return ident;
}

/* Used for implied declarations of nets */

tree make_default_net(tree ident)
{
    tree spec = make_net_spec(default_net_type, NULL_TREE, NULL_TREE);
    tree decl = make_decl(ident, spec, NULL_TREE, NULL_TREE);

    BLOCK_DECL(current_scope) = chainon(decl, BLOCK_DECL(current_scope));
    REFERENCED_ATTR(decl) = 1;
    return decl;
}

/* Make a TMP_DECL node for statements that use intermediate values */

static tree tmp_decl_free = NULL;

tree make_tmp_decl(tree lval)
{
    tree tmp;
    if (tmp_decl_free) {
	tmp = tmp_decl_free;
	tmp_decl_free = DECL_THREAD(tmp_decl_free);
	DECL_THREAD(tmp) = NULL;
    } else {
	tmp = make_node(TMP_DECL);
    }
    TREE_CHAIN(tmp) = lval;
    pass3_decl(tmp);
    return tmp;
}

void free_tmp_decl(tree tmp)
{
    free(DECL_STORAGE(tmp));
    DECL_THREAD(tmp) = tmp_decl_free;
    tmp_decl_free = tmp;
}

tree root_port_decl(tree node)
{
    switch (TREE_CODE(node)) {
    case PART_REF:
	return PART_DECL(node);
    case BIT_REF:
	return BIT_REF_DECL(node);
    case IDENTIFIER_NODE:
	return IDENT_CURRENT_DECL(node);
    }
    return node;
}

tree root_port_name(tree node)
{
    switch (TREE_CODE(node)) {
    case PART_REF:
	return PART_NAME(node);
    case BIT_REF:
	return BIT_REF_NAME(node);
    case IDENTIFIER_NODE:
	return node;
    }
    return node;
}

/**************************************************************
 *
 * init_decl
 *	- initialize global and static variables in decl.c
 *
 **************************************************************
 */

void init_decl()
{
    tmp_decl_free = NULL;
    integer_zero_node = NULL_TREE;
    integer_one_node = NULL_TREE;
    bit_zero_node = NULL_TREE;
    bit_one_node = NULL_TREE;
    error_mark_node = NULL_TREE;
}

/**************************************************************
 *
 * make_port_decl
 *	- process port definition in 2001 format
 *
 **************************************************************
 */

struct port_node *add_port (struct port_node *head)
{
	struct port_node *p;

	p = (struct port_node *) malloc ( sizeof (struct port_node) );
	if ( p )
    {
		//printf("Memory allocated at: %x\n",p);
        p->next = head;
        p->id = NULL_TREE;
    }
	else
		printf("Not Enough Memory in add_port()\n");

    return p;
}

struct port_array_node *add_port_array (struct port_array_node *head)
{
	struct port_array_node *p;

	p = (struct port_array_node *) malloc ( sizeof (struct port_array_node) );
	if ( p )
    {
		//printf("Memory allocated at: %x\n",p);
        p->next = head;
        p->in_or_out = 0;
        p->wire_or_reg = 0;
        //p->range = NULL_TREE;
        p->port = NULL;
    }
	else
		printf("Not Enough Memory in add_array_node()\n");

    return p;
}


int make_port_decl (struct port_array_node *head)
{
    int n = 0 ;

    struct port_array_node *array, *arr_next;
	struct port_node *p;
    array = head;
    p = array->port;

    if ( head == NULL || p == NULL )
    {
		printf("Head is NULL in make_port_decl()\n");
        return -1;
    }

    tree decl_t;
    tree curr_spec_t;

     while ( array )
     {
        if ( array->wire_or_reg == 1 )
            curr_spec_t = ( array->is_bus == 0 ) ? 
                          make_reg_spec ( NULL_TREE ) :
                          make_reg_spec ( &(array->range) );
        else
            curr_spec_t = ( array->is_bus == 0 ) ?
                          make_net_spec (default_net_type, NULL_TREE, NULL_TREE) :
                          make_net_spec (default_net_type, &(array->range), NULL_TREE);

        if ( array->in_or_out == 0 )
            PORT_INPUT_ATTR (curr_spec_t) = 1;
        else if ( array->in_or_out == 1 )
            PORT_OUTPUT_ATTR (curr_spec_t) = 1;
        else
        {
            PORT_INPUT_ATTR (curr_spec_t) = 1;
            PORT_OUTPUT_ATTR (curr_spec_t) = 1;
        }

        //---------------------------------------------
        // begin in each array

        p = array->port;
        array->port = p->next;
		decl_t = make_decl (check_port (p->id), curr_spec_t, NULL_TREE, NULL_TREE);
        //printf("Memory free at %x\n", p);
        free (p);
        n++;

        while ( array->port )
        {
            p = array->port;
            array->port = p->next;
            decl_t = chainon (make_decl (check_port (p->id), curr_spec_t, NULL_TREE, NULL_TREE), decl_t);
            //printf("Memory free at %x\n", p);
            free (p);
            n++;
        }

		BLOCK_PORTS (current_scope) =
		    chainon (decl_t, BLOCK_PORTS (current_scope));

        // end in each array
        //---------------------------------------------

        arr_next = array->next;
        //printf("Memory free at %x\n", array);
        free (array);
        array = arr_next;
	}

	return n;
}


/**************************************************************
 *
 * reg_init()
 *	- process reg initialization in 2001 format
 *
 **************************************************************
 */
struct reg_node *add_reg (struct reg_node *head)
{
	struct reg_node *p;

	p = (struct reg_node *) malloc ( sizeof (struct reg_node) );
	if ( p )
    {
		//printf("Memory allocated at: %x\n",p);
        p->next = head;
        p->id = NULL_TREE;
    }
	else
		printf("Not Enough Memory in add_reg()\n");

    return p;
}


int make_reg_init (struct reg_node *head, tree curr_module, tree curr_spec, lineno_t line_no)
{
    if ( head == NULL )
    {
		//printf("Head is NULL in reg_init()\n");
        return -1;
    }

	struct reg_node *p, *pn;

    p = head;

    tree id_t;

    while ( p )
    {
		id_t = check_lval (p->id, LVAL_REG, curr_spec);

		id_t = build_stmt (ASSIGN_STMT, line_no, id_t,
				implicit_conversion (id_t, p->value) );

		id_t = build_stmt (INITIAL_BLOCK, line_no, id_t);
        
        BLOCK_BODY (curr_module) = tree_cons (id_t,
			(tree)INITIAL_CODE, BLOCK_BODY (curr_module));

        //printf("Memory free at %x\n", p);
        pn = p->next;
        free ( p );
        p = pn;
	}

	return 0;
}

