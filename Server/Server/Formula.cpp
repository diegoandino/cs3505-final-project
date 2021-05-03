#include "Formula.h"

Formula::Formula() : formula("") {}


Formula::Formula(std::string& formula)
{

	//initialization

	//keep track of the paretheses count
	int rightParCount = 0;
	int leftParCount = 0;
	//track follow rules
	bool followsOpeningPar = false;
	bool followsOperator = false;

	tokens = get_tokens(formula);

	//first checks

	if (tokens.size() == 0)
	{
		std::cout << "Formula is empty" << std::endl;
		set_error();
		return;
	}

	if (!op_par_follow_rule(tokens[0]))
	{
		std::cout << "Invalid token at the beginning of the expression" << std::endl;
		set_error();
		return;
	}

	//Following rules and balanced parantheses

	for (int i = 0; i < tokens.size(); i++)
	{
		//this way tokens can be verified
		//in case there is no isValid method passed
		if (!token_is_valid(tokens[i]))
		{
			std::cout << "Invalid token at the beginning of the expression" << std::endl;
			set_error();
			return;
		}

		//Open parenthesis/Operator Following Rule
		//checks the token after an opening parentheses & operators
		if (followsOpeningPar || followsOperator)
		{
			if (!op_par_follow_rule(tokens[i]))
			{
				std::cout << "Invalid token at the beginning of the expression" << std::endl;
				set_error();
				return;
			}

			followsOpeningPar = false;
			followsOperator = false;

		}

		//Extra Following Rule
		// this considers that if the token is not an opening parentheses or an operator
		// then it must be an extra token
		else
		{
			if (!(is_operator(tokens[i]) || tokens[i] == ")") && i != 0)
			{
				std::cout << "a token is not valid" << std::endl;
				set_error();
				return;
			}
		}

		//keeps track of the parentheses' count
		if (tokens[i] == "(")
		{
			leftParCount++;
			followsOpeningPar = true;
		}

		//checks for balanced parentheses
		else if (tokens[i] == ")")
		{
			rightParCount++;
			if (rightParCount > leftParCount)
			{
				std::cout << "\')\' misssing " << std::endl;
				set_error();
				return;
			}
		}
		else if (is_operator(tokens[i]))
		{
			followsOperator = true;
		}

		//adds a normalized double to the the tokens array
		std::string::size_type sz;
		try
		{
			double d = std::stod(tokens[i], &sz);
			tokens[i] = std::to_string(d);
		}
		catch (const std::exception&)
		{
			if (is_variable(tokens[i]))
			{
				
					std::cout << "a token is not valid" << std::endl;
					set_error();
					return;
				
			}
		}
	}

	//last checks

	try
	{
		std::string::size_type sz;
		double d = std::stod(tokens[tokens.size() - 1], &sz);
	}
	catch (const std::exception&)
	{
		if (!(is_variable(tokens[tokens.size() - 1]) || tokens[tokens.size() - 1] == ")"))
		{
			std::cout << "Invalid token at the end of the expression" << std::endl;
			set_error();
			return;
		}
	}


	if (rightParCount != leftParCount)
	{
		std::cout << "parantheses missing" << std::endl;
		set_error();
		return;
	}
}

CellValue Formula::evaluate()
{
	//creates the two stacks, for operators and for values
	std::stack<std::string> operator_stack;
	std::stack<double> value_stack;
	
	std::string add("+");
	std::string sub("-");
	std::string mult("*");
	std::string div("/");

	for (std::string t : tokens)
	{
		//trim(t);

		try
		{
			std::string::size_type sz;
			double d = std::stod(t, &sz);

			if (!perform_div_mult(value_stack, operator_stack, d))
			{
				std::cout << "Division by zero ocurred" << std::endl;
				CellValue c;
				c.set_error();
				return c;
			}

		}

		catch (const std::exception&)
		{
			//t is a variable
			if (is_variable(t))
			{
				try
				{
					double CurrVar = lookup(t);

					if (!perform_div_mult(value_stack, operator_stack, CurrVar))
					{
						std::cout << "Division by zero ocurred" << std::endl;
						CellValue c;
						c.set_error();
						return c;
					}
				}
				catch (const std::exception&)
				{
					std::cout << "Invalid Variable" << std::endl;
					CellValue c;
					c.set_error();
					return c;
				}
			}

			//t is + or -
			else if (t == ("+") || t == ("-"))
			{
				if (operator_is_on_top(operator_stack, add) || operator_is_on_top(operator_stack, sub))
				{
					perform_add_subs(value_stack, operator_stack);
				}

				operator_stack.push(t);
			}
			//t is * or  / or (
			else if (t == "*" || t == "/" || t == "(")
			{
				operator_stack.push(t);
			}
			//t is )
			else if (t == ")")
			{
				if (operator_is_on_top(operator_stack, add) || operator_is_on_top(operator_stack, sub))
				{
					perform_add_subs(value_stack, operator_stack);
				}

				//this should be an '('
				operator_stack.pop(); // if you're wondering why, it's because in c++ pop() deletes but doens't return anything....

				if (operator_is_on_top(operator_stack, mult) || operator_is_on_top(operator_stack, div))
				{
					double firs_num = value_stack.top();
					value_stack.pop();
					double second_num = value_stack.top();
					value_stack.pop();
					std::string curr_operator = operator_stack.top();
					operator_stack.pop();

					if (curr_operator == "*")
					{
						value_stack.push(firs_num * second_num);
					}
					else if (curr_operator == "/")
					{
						if (firs_num == 0)
						{
							std::cout << "Division by zero occured" << std::endl;
							CellValue c;
							c.set_error();
							return c;
						}

						value_stack.push(second_num / firs_num);
					}
				}
			}
		}
	}

	//processes the last token

	double ret = 0;

	// first case
	if (operator_stack.size() == 0)
	{
		ret = value_stack.top();
		value_stack.pop();
	}
	//second case
	else if (operator_stack.size() != 0)
	{
		double first_num = value_stack.top();
		value_stack.pop();
		double second_num = value_stack.top();
		value_stack.pop();
		std::string curr_operator = operator_stack.top();
		operator_stack.pop();

		if (curr_operator == "+")
		{
			ret = first_num + second_num;
		}
		else if (curr_operator == "-")
		{
			ret = second_num - first_num;
		}
	}

	std::string s = std::to_string(ret);
	CellValue cell_value(s);
	return cell_value;
}

/// <summary>
/// Enumerates the normalized versions of all of the variables that occur in this 
/// formula.  No normalization may appear more than once in the enumeration, even 
/// if it appears more than once in this Formula.
/// 
/// For example, if N is a method that converts all the letters in a string to upper case:
/// 
/// new Formula("x+y*z", N, s => true).GetVariables() should enumerate "X", "Y", and "Z"
/// new Formula("x+X*z", N, s => true).GetVariables() should enumerate "X" and "Z".
/// new Formula("x+X*z").GetVariables() should enumerate "x", "X", and "z".
/// </summary>
std::vector<std::string> Formula::get_variables()
{
	std::vector<std::string> variables;

	for (std::string s : tokens)
	{
		if (is_variable(s))
		{
			variables.push_back(s);
		}
	}

	return variables;
}

/// <summary>
/// Returns a string containing no spaces which, if passed to the Formula
/// constructor, will produce a Formula f such that this.Equals(f).  All of the
/// variables in the string should be normalized.
/// 
/// For example, if N is a method that converts all the letters in a string to upper case:
/// 
/// new Formula("x + y", N, s => true).ToString() should return "X+Y"
/// new Formula("x + Y").ToString() should return "x+Y"
/// </summary>
std::string Formula::to_string()
{
	std::string ret;
	for (std::string s : tokens)
	{
		ret += s;
	}

	return ret;
}

bool Formula::equals(Formula& other)
{
	//check for null
	/*if (other == NULL)
		return false;*/

	return this->to_string() == other.to_string();
}

/// <summary>
/// Given an expression, enumerates the tokens that compose it.  Tokens are left paren;
/// right paren; one of the four operator symbols; a string consisting of a letter or underscore
/// followed by zero or more letters, digits, or underscores; a double literal; and anything that doesn't
/// match one of those patterns.  There are no empty tokens, and no token contains white space.
/// </summary>
std::vector<std::string> Formula::get_tokens(std::string& formula)
{

	std::string pattern = "([\\+\\-])|(\\()|(\\))|([0-9]+\\.?[0-9]*)|([\\*\\/])|[a-zA-Z]?[0-9]*";
	std::regex rgx(pattern);

	std::vector<std::string> ret;

	std::regex_iterator<std::string::iterator> rit(formula.begin(), formula.end(), rgx);
	std::regex_iterator<std::string::iterator> rend;

	while (rit != rend)
	{
		if(rit->str() != "")
			ret.push_back(rit->str());
		++rit;
	}

	return ret;
}


/// <summary>
/// Checks if a given token is valid
/// </summary>
/// <param name="token"></param>
/// <returns></returns>
bool Formula::token_is_valid(std::string& token)
{

	//if it is not any of the allowed character returns false
	if (token == "(" ||
		token == ")" ||
		token == "+" ||
		token == "-" ||
		token == "*" ||
		token == "/" ||
		is_variable(token))
	{
		return true;
	}

	try
	{
		std::string::size_type sz;
		double d = std::stod(token, &sz);

		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}

	return false;
}

/// <summary>
/// checks if a Token is either a number, variable or opening parenthese "("
/// </summary>
/// <param name="s"></param>
/// <returns>true if the token is either of the three</returns>
bool Formula::op_par_follow_rule(std::string& s)
{

	try
	{
		std::string::size_type sz;
		double d = std::stod(s, &sz);

		return true;
	}
	catch (const std::exception&)
	{
		return is_variable(s) || s == "(";
	}
	
}

/// <summary>
/// checks if a string s is one of the four operator symbols
/// </summary>
/// <param name="s"></param>
/// <returns>true if the given string is an operator</returns>
bool Formula::is_operator(std::string& s)
{
	return s == "+" || s == "-" || s == "*" || s == "/";
}

/// <summary>
/// Performs either division or multiplication to the give number and pushes the result to the Value Stack
/// </summary>
/// <param name="Values"></param>
/// <param name="Operators"></param>
/// <param name="Num"></param>
bool Formula::perform_div_mult(std::stack<double>& values, std::stack<std::string>& operators, double num)
{
	std::string div("/");
	std::string mult("*");
	if (operator_is_on_top(operators, div) || operator_is_on_top(operators, mult))
	{
		double curr_num = values.top();
		values.pop();
		std::string op = operators.top();
		operators.pop();

		if (op == ("*"))
		{
			values.push(num * curr_num);
		}
		else if (op == ("/"))
		{
			if (num == 0)
				return false;

			values.push(curr_num / num);
		}
	}
	else
	{
		values.push(num);
	}

	return true;
}

/// <summary>
/// Performs either addition or substraction pushes the result to the Value Stack
/// </summary>
/// <param name="Values"></param>
/// <param name="Operators"></param>
/// <param name="Num"></param>
void Formula::perform_add_subs(std::stack<double>& values, std::stack<std::string>& operators)
{
	double first_num = values.top();
	values.pop();
	double second_num = values.top();
	values.pop();
	std::string curr_op = operators.top();
	operators.pop();

	if (curr_op == ("+"))
	{
		values.push(first_num + second_num);
	}
	else if (curr_op == ("-"))
	{
		values.push(second_num - first_num);
	}
}


/// <summary>
/// checks if a given token is a varible
/// </summary>
/// <param name="s"></param>
bool Formula::is_variable(std::string& s)
{
	std::string varPattern = "[a-zA-Z]?[0-9]*";

	return std::regex_match(s, std::regex(varPattern));
}

/// <summary>
/// Peeks safely on a stack to see if a specific token is on top
/// </summary>
/// <typeparam name="T"></typeparam>
/// <param name="stack"></param>
/// <param name="component"></param>
/// <returns>true if the desired component is on top</returns>
bool Formula::operator_is_on_top(std::stack<std::string>& stack, std::string& tkn)
{
	if (stack.size() < 1)
		return false;

	return stack.top() == (tkn);
}

void Formula::set_error()
{
	tokens.clear();
	tokens.push_back("FORMULA::ERROR");
	
}

bool Formula::is_Error() {
	return to_string() == "FORMULA::ERROR";
}

bool Formula::is_valid(std::string& str)
{
	return true;
}

double Formula::lookup(std::string& str)
{
	return 0.0;
}


//from: https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring

// trim from start (in place)
inline void Formula::ltrim(std::string& s)
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch)
		{
			return !std::isspace(ch);
		}));
}

// trim from end (in place)
inline void Formula::rtrim(std::string& s)
{
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch)
		{
			return !std::isspace(ch);
		}).base(), s.end());
}

// trim from both ends (in place)
inline void Formula::trim(std::string& s)
{
	ltrim(s);
	rtrim(s);
}
