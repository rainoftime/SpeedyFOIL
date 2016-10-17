
#include <z3++.h>
#include <z3_fixedpoint.h>

int main(){


  // z3::set_param("engine", "datalog");

  z3::context c;


  z3::expr_vector ev(c); // this should be defined before fp, otherwise will crash


  Z3_fixedpoint fp = Z3_mk_fixedpoint( c );
  

  z3::sort bv3 = c.bv_sort(3);
  z3::sort bl = c.bool_sort();
  z3::func_decl edge = c.function("edge", bv3, bv3, bl);
  z3::func_decl path = c.function("path", bv3, bv3, bl);
  z3::func_decl p = c.function("p", bv3, bl);
  

  z3::expr va = c.constant("a", bv3);
  z3::expr vb = c.constant("b", bv3);
  z3::expr vc = c.constant("c", bv3);
  
  Z3_fixedpoint_register_relation(c, fp, edge);
  Z3_fixedpoint_register_relation(c, fp, path);
  Z3_fixedpoint_register_relation(c, fp, p);

  //z3::expr p_va_vb = path(va,vb);
  z3::expr r1 = z3::forall(va, vb, z3::implies( edge(va,vb), path(va,vb) ));

  //z3::expr r1 = z3::forall(va, vb, z3::implies( edge(va,vb), p(va) ));
  //z3::expr r1 = z3::implies( edge(va,vb), p(va) );

  //z3::expr r2 = z3::forall(va, vb, z3::implies( edge(va,vb), p(vb) ));


  ev.push_back( edge(va,vb) );
  ev.push_back( path(vb,vc) );
  z3::expr r2 = z3::forall(va, vb, vc, z3::implies( z3::mk_and(ev), path(va,vc) ));
  //z3::expr r2 = z3::implies( z3::mk_and(ev), path(va,vc) );

  //z3::expr r2 = z3::forall(va, vb, vc, z3::implies(  edge(va,vb) && path(vb,vc) , path(va,vc) ));
  //z3::expr r2 = z3::forall(va, vb, vc, z3::implies( edge(va,vb), path(va,vc) ));




  z3::symbol s_r1 = c.str_symbol("r1");
  z3::symbol s_r2 = c.str_symbol("r2");

  //Z3_fixedpoint_inc_ref(c, fp);

  Z3_fixedpoint_add_rule(c, fp, r1, s_r1);
  Z3_fixedpoint_add_rule(c, fp, r2, s_r2);


  //Z3_fixedpoint_add_rule(c, fp, r2, s_r2);


  z3::expr v1 = c.bv_val(1, 3);
  z3::expr v2 = c.bv_val(2, 3);
  z3::expr v3 = c.bv_val(3, 3);
  z3::expr v4 = c.bv_val(4, 3);

  z3::symbol name = c.str_symbol("");
  Z3_fixedpoint_add_rule(c, fp, edge(v1, v2), name);
  Z3_fixedpoint_add_rule(c, fp, edge(v1, v3), name);
  Z3_fixedpoint_add_rule(c, fp, edge(v2, v4), name);



  //Z3_lbool res = Z3_fixedpoint_query (c, fp, path(v1,v4)); 
  //Z3_lbool res = Z3_fixedpoint_query (c, fp, path(v1,v3));
  Z3_lbool res = Z3_fixedpoint_query (c, fp, z3::exists(va,vb,path(va,vb)) );
  //Z3_lbool res = Z3_fixedpoint_query (c, fp, z3::forall(va,vb,path(va,vb)) );

  //Z3_lbool res = Z3_fixedpoint_query (c, fp, path(va,vb)); 
  // Z3_ast_vector v_res = Z3_fixedpoint_get_rules(c, fp);
  //int sz = Z3_ast_vector_size(c, v_res);
  //std::cout << "v_res.size = " << sz << std::endl;


  Z3_ast ast_res = Z3_fixedpoint_get_answer(c,fp);
  z3::ast detailed_res (c, ast_res);
  
  std::cout << detailed_res << std::endl;



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
