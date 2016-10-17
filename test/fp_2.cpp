
#include <z3++.h>
#include <z3_fixedpoint.h>

int main(){

  z3::context c;

  Z3_fixedpoint fp = Z3_mk_fixedpoint( c );
  

  z3::expr x = c.int_const("x");
  std::cout << x + 1 << "\n";


  
  z3::expr va = c.bool_const("a");
  z3::expr vb = c.bool_const("b");
  z3::expr vc = c.bool_const("c");
  
  z3::func_decl fun_a = va.decl();  
  z3::func_decl fun_b = vb.decl();  
  z3::func_decl fun_c = vc.decl();  

  // Z3_fixedpoint_inc_ref(c, fp);

  Z3_fixedpoint_register_relation(c, fp, fun_a);
  Z3_fixedpoint_register_relation(c, fp, fun_b);
  Z3_fixedpoint_register_relation(c, fp, fun_c);

  z3::expr r1 = z3::implies(vb, va); // a :- b
  z3::expr r2 = z3::implies(vc, vb); // b :- c

  z3::symbol s_r1 = c.str_symbol("r1");
  z3::symbol s_r2 = c.str_symbol("r2");

  Z3_fixedpoint_add_rule(c, fp, r1, s_r1);
  Z3_fixedpoint_add_rule(c, fp, r2, s_r2);

  Z3_fixedpoint_add_fact(c, fp, fun_c, 0, nullptr);

  Z3_lbool res = Z3_fixedpoint_query (c, fp, va); // crashed, when mistakenly put va -> fa

  if(res == Z3_L_FALSE) {
    std::cout << "res is L_false" << std::endl;
  }
  else if(res == Z3_L_UNDEF) {
    std::cout << "res is L_undef" << std::endl;
  }
  else if(res == Z3_L_TRUE) {
    std::cout << "res is L_true" << std::endl;
  }
  else {
    std::cout << "res is unknown value" << std::endl;
  }


  return 0;
}
