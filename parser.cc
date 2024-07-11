#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include "compiler.h"
#include "lexer.h"
#include <vector>
#include <iostream>

using namespace std;

LexicalAnalyzer lexer;
Token token;

#include <map>

map<string, int> variables;

int location(string name)
{
    if (variables.count(name) > 0)
    {
        return variables[name];
    }
    return -1; // or throw an exception if variable not found
}

/* 
    NOTE: using assignment_basic1, this is what your structures should look like
    variables     mem
     i : 0         0      variables map -> variable name : index of the variable's location in the memory
     j : 1         0      mem array -> value of the coreesponding variable
*/
void parse_var_section()
{
     token = lexer.peek(1); // peek so the first ID is not consumed

     while (token.token_type != SEMICOLON)       
     {                                           
          token = lexer.GetToken();             
          if (token.token_type == ID && token.lexeme.length() == 1)
          {
                variables[token.lexeme] = next_available; //static_cast<int>(token.lexeme[0]); // Convert single character to its ASCII value
                mem[next_available] = 0;
                next_available++;
          }
          else if (token.token_type == COMMA)
          {
                continue;
          }
     }
}

// write the parse_input_stmt
InstructionNode *parse_input_stmt()
{
   // cout << "IN" << endl;
    InstructionNode *inst = new InstructionNode();
    inst->next = nullptr;
    // token = lexer.GetToken(); // consume 'INPUT'     NOTE: INPUT was consumed in parse_stmt
    token = lexer.GetToken(); // consume variable identifier
    inst->type = IN;
    inst->input_inst.var_index = location(token.lexeme);
    
   // cout << "Parsing INPUT statement" << endl;
    token = lexer.GetToken(); // consume ';'            
    return inst;
}

void parse_input()
{
    token = lexer.GetToken();
    while (token.token_type == NUM || token.token_type == COMMA)
    {
        if (token.token_type == NUM)
        {
            variables[token.lexeme] = stoi(token.lexeme);        
            inputs.push_back(stoi(token.lexeme));
            token = lexer.GetToken();
        }
    }
}

InstructionNode *parse_output_stmt()
{
    InstructionNode *inst = new InstructionNode();
    inst->next = nullptr;
    //token = lexer.GetToken();
   // cout << "Token found: " << token.lexeme << endl; // consume 'OUTPUT'
    token = lexer.GetToken();
 //   cout << "Token found: " << token.lexeme << endl; // consume variable identifier
    inst->type = OUT;
    inst->output_inst.var_index = location(token.lexeme);
    
//    cout << "Parsing OUTPUT statement" << endl; // Add cout statement for tracking
    token = lexer.GetToken(); // consume ';'
    return inst;
}


struct InstructionNode *parse_if_stmt()
{
    InstructionNode *inst = new InstructionNode();
    inst->next = nullptr;
    inst->type = InstructionType(CJMP);

    // Parse the conditional expression
   
    token = lexer.GetToken(); // consume the first operand
    // Check if the second operand is an ID
    if (token.token_type == ID)
    {
        inst->assign_inst.operand1_index = location(token.lexeme);
    }
    // If it's not an ID, check if it's a NUM
    else if (token.token_type == NUM)
    {
        // Store the NUM in operand2_index
        variables[token.lexeme] = next_available;
        mem[next_available] = stoi(token.lexeme);
        inst->assign_inst.operand1_index = next_available;
        next_available++;
    }

    // Parse the conditional operator
    token = lexer.GetToken(); // consume the conditional operator
    if (token.token_type == GREATER)
    {
        inst->cjmp_inst.condition_op = ConditionalOperatorType(CONDITION_GREATER);
    }
    else if (token.token_type == LESS)
    {
        inst->cjmp_inst.condition_op = ConditionalOperatorType(CONDITION_LESS);
    }
    else if (token.token_type == NOTEQUAL)
    {
        inst->cjmp_inst.condition_op = ConditionalOperatorType(CONDITION_NOTEQUAL);
    }

    // Parse the second operand
    token = lexer.GetToken(); // consume the second operand
    // Check if the second operand is an ID
    if (token.token_type == ID)
    {
        inst->assign_inst.operand2_index = location(token.lexeme);
    }
    // If it's not an ID, check if it's a NUM
    else if (token.token_type == NUM)
    {
        // Store the NUM in operand2_index
        variables[token.lexeme] = next_available;
        mem[next_available] = stoi(token.lexeme);
        inst->assign_inst.operand2_index = next_available;
        next_available++;
    }

    // Create a new target instruction node
    InstructionNode *target = new InstructionNode();
    target->type = InstructionType(NOOP);
    target->next = nullptr;

    // Set the target instruction node for the conditional jump instruction
    inst->cjmp_inst.target = target;
    InstructionNode *parse_stmt_list();
    // Parse the statement block for the if statement
    token = lexer.GetToken(); // consume '{'
    // InstructionNode *stmt_block = parse_stmt_list();
    // token = lexer.peek(1); // peek to see if it is a RBRACE or a nested IF
    // if (token.token_type == RBRACE)
    // {
    //     token = lexer.GetToken(); // consume '}'
    // } else if(token.token_type == IF){
    //     parse_if_stmt();
    // }

    // Set the next instruction node after the if statement block
    // stmt_block->next = target;

    // Set the next instruction node after the conditional jump instruction
    inst->next = parse_stmt_list();

    // insert target after the last instruction in the if block
    InstructionNode *temp = inst->next;
    while (temp->next != nullptr)
    {
        temp = temp->next;
    }
    temp->next = target;

    Token t = lexer.GetToken();

    return inst;
}


struct InstructionNode *parse_while_stmt()
{
    InstructionNode *inst = new InstructionNode();
    inst->type = InstructionType(CJMP);

    // Parse the conditional expression
 
    token = lexer.GetToken(); // consume the first operand
    inst->cjmp_inst.operand1_index = location(token.lexeme);

    // Parse the conditional operator
    token = lexer.GetToken(); // consume the conditional operator
    if (token.token_type == GREATER)
    {
        inst->cjmp_inst.condition_op = ConditionalOperatorType(CONDITION_GREATER);
    }
    else if (token.token_type == LESS)
    {
        inst->cjmp_inst.condition_op = ConditionalOperatorType(CONDITION_LESS);
    }
    else if (token.token_type == NOTEQUAL)
    {
        inst->cjmp_inst.condition_op = ConditionalOperatorType(CONDITION_NOTEQUAL);
    }

    // Parse the second operand
    token = lexer.GetToken(); // consume the second operand
    inst->cjmp_inst.operand2_index = location(token.lexeme);

    // Parse the closing parenthesis
   

    // Create a new target instruction node
    InstructionNode *target = new InstructionNode();
    target->type = InstructionType(NOOP);
    target->next = nullptr;

    // Set the target instruction node for the conditional jump instruction
    inst->cjmp_inst.target = target;
    InstructionNode *parse_stmt_list();
    // Parse the statement block for the while statement
    token = lexer.GetToken(); // consume '{'
    InstructionNode *stmt_block = parse_stmt_list();
    token = lexer.GetToken(); // consume '}'

    // Set the next instruction node after the while statement block
    stmt_block->next = inst;

    // Set the next instruction node after the conditional jump instruction
    inst->next = target;

    return stmt_block;
}


struct InstructionNode *parse_assign_stmt()
{
    InstructionNode *inst = new InstructionNode();
    inst->next = nullptr;
    inst->type = ASSIGN;
    inst->assign_inst.left_hand_side_index = location(token.lexeme);

    token = lexer.GetToken(); // consume '='
    token = lexer.GetToken(); // consume the first operand
    if (token.token_type == ID)
    {
        inst->assign_inst.operand1_index = location(token.lexeme);
    }
    // If it's not an ID, check if it's a NUM
    else if (token.token_type == NUM)
    {
        // Store the NUM in operand2_index
        variables[token.lexeme] = next_available;
        mem[next_available] = stoi(token.lexeme);
        inst->assign_inst.operand1_index = next_available;
        next_available++;
    }

    token = lexer.GetToken(); // consume the arithmetic operator
    if (token.token_type == PLUS)
    {
        inst->assign_inst.op = OPERATOR_PLUS;
    }
    else if (token.token_type == MINUS)
    {
        inst->assign_inst.op = OPERATOR_MINUS;
    }
    else if (token.token_type == MULT)
    {
        inst->assign_inst.op = OPERATOR_MULT;
    }
    else if (token.token_type == DIV)
    {
        inst->assign_inst.op = OPERATOR_DIV;
    }
    else
    {
        // Handle the case when no operator is provided
        inst->assign_inst.op = OPERATOR_NONE;
        // Add a return statement here to prevent consumption of the second operand
        return inst;
    }

    token = lexer.GetToken(); // consume the second operand

    // Check if the second operand is an ID
    if (token.token_type == ID)
    {
        inst->assign_inst.operand2_index = location(token.lexeme);
    }
    // If it's not an ID, check if it's a NUM
    else if (token.token_type == NUM)
    {
        // Store the NUM in operand2_index
        variables[token.lexeme] = next_available;
        mem[next_available] = stoi(token.lexeme);
        inst->assign_inst.operand2_index = next_available;
        next_available++;
    }
    // Otherwise, handle the error or unexpected case

   // cout << "Parsed assignment statement" << endl; // tracking statement

    token = lexer.GetToken();
    return inst;
}

struct InstructionNode *parse_stmt()
{
    InstructionNode *inst = new InstructionNode();
    inst->next = nullptr;
    token = lexer.GetToken(); // consume the first token
   // cout << "Token found: " << token.lexeme << endl;
    if (token.token_type == INPUT)
    {
        inst = parse_input_stmt();
    }
    else if (token.token_type == OUTPUT)
    {
        inst = parse_output_stmt();
    }
   
    else if (token.token_type == ID)
    {
        inst = parse_assign_stmt();
    }
    else if (token.token_type == IF)
    {
        inst = parse_if_stmt();
    }
    else if (token.token_type == WHILE)
    {
        inst = parse_while_stmt();
    }
    

    return inst;
}

InstructionNode *parse_stmt_list()
{
    InstructionNode *inst1 = new InstructionNode();
    InstructionNode *inst2 = new InstructionNode();

    inst1 = parse_stmt();
    Token t = lexer.peek(1);
    if (t.token_type == RBRACE)
    {
      //  cout << "Reached end of statement list" << endl; // tracking statement
     //   cout << "inst1: " << inst1 << endl;

        return inst1;
    }

    else
    {
        inst2 = parse_stmt_list();
    }

    InstructionNode *temp = inst1;

    while (temp->next != nullptr)
    {
        //cout << "IN" << endl;
        temp = temp->next;
    }
   // cout << "OUT" << endl;
    temp->next = inst2;

   // cout << "Parsed statement list" << endl; // tracking statement

    // returning head of linked list
a    return inst1;
}

InstructionNode *parse_body()
{
    token = lexer.GetToken(); // consume '{'
    InstructionNode *stmt_lthist = parse_stmt_list();
    token = lexer.GetToken(); // consume '}'

    return stmt_lthist;
}

InstructionNode *parse_program()
{
    parse_var_section();
    InstructionNode *head = parse_body();
    parse_input();
    return head;
}



struct InstructionNode *parse_generate_intermediate_representation()
{
    InstructionNode *head = parse_program();
    
  //  cout << "DONE" << endl;

    return head;
}