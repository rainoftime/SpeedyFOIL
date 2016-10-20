
#include <z3++.h>

#define USE_BIT_VEC

int main(){


  z3::set_param("fixedpoint.engine", "datalog"); // int sort will crash
  //z3::set_param("fixedpoint.engine", "pdr"); // default engine for int sort


  z3::context c;


  Z3_fixedpoint fp = Z3_mk_fixedpoint( c );
  Z3_fixedpoint_inc_ref(c,fp); 

  // debug experiences show that this should be defined before fp, otherwise code will crash
  // such weird issue got resolved by placing Z3_fixedpoint_inc_ref/Z3_fixedpoint_dec_ref at proper locations
  z3::expr_vector ev(c); 


  

#ifdef USE_BIT_VEC
  z3::sort bv3 = c.bv_sort(3);
#else
  z3::sort bv3 = c.int_sort();
#endif

  z3::sort bl = c.bool_sort();
  z3::func_decl edge = c.function("edge", bv3, bv3, bl);
  z3::func_decl path = c.function("path", bv3, bv3, bl);
  

  z3::expr va = c.constant("a", bv3);
  z3::expr vb = c.constant("b", bv3);
  z3::expr vc = c.constant("c", bv3);
  
  Z3_fixedpoint_register_relation(c, fp, edge);
  Z3_fixedpoint_register_relation(c, fp, path);


  z3::expr r1 = z3::forall(va, vb, z3::implies( edge(va,vb), path(va,vb) ));

  ev.push_back( edge(va,vb) );
  ev.push_back( path(vb,vc) );
  z3::expr r2 = z3::forall(va, vb, vc, z3::implies( z3::mk_and(ev), path(va,vc) ));


  z3::symbol s_r1 = c.str_symbol("r1");
  z3::symbol s_r2 = c.str_symbol("r2");

  Z3_fixedpoint_add_rule(c, fp, r1, s_r1);
  Z3_fixedpoint_add_rule(c, fp, r2, s_r2);


#ifdef  USE_BIT_VEC

  z3::expr v1 = c.bv_val(1, 3);
  z3::expr v2 = c.bv_val(2, 3);
  z3::expr v3 = c.bv_val(3, 3);
  z3::expr v4 = c.bv_val(4, 3);

#else

  z3::expr v1 = c.int_val(1);
  z3::expr v2 = c.int_val(2);
  z3::expr v3 = c.int_val(3);
  z3::expr v4 = c.int_val(4);
#endif


  z3::symbol name = c.str_symbol("");
  Z3_fixedpoint_add_rule(c, fp, edge(v1, v2), name);
  Z3_fixedpoint_add_rule(c, fp, edge(v1, v3), name);
  Z3_fixedpoint_add_rule(c, fp, edge(v2, v4), name);


  Z3_lbool res = Z3_fixedpoint_query (c, fp, z3::exists(va,vb,path(va,vb)) );

  std::cout << "res = " << res << std::endl;
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


  Z3_ast ast_res = Z3_fixedpoint_get_answer(c,fp);
  //z3::ast detailed_res (c, ast_res);
  z3::expr detailed_res (c, ast_res);
   
  if(detailed_res.is_app()) {
    std::cout << "detailed_res is app" << std::endl;

    int n_args = detailed_res.num_args();
    std::cout << "number of args: " << n_args << std::endl;
    for(int i=0; i < n_args; ++i) {
      std::cout << i << "-th arg: " << detailed_res.arg(i) << std::endl;
    }

  }
  std::cout << detailed_res << std::endl;

  Z3_fixedpoint_dec_ref(c,fp);


  return 0;
}
