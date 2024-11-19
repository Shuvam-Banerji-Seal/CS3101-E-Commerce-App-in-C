

// Cart management///need to debug
bool update_cart_quantity(int product_id,  int quantity) {
    if (!db) return false;

    sqlite3_stmt *stmt1;
    const char* sql1= "SELECT stock FROM products WHERE product_id = ? ;";
    int stock_count;
    if(sqlite3_prepare_v2(db , sql1,-1,&stmt1,NULL)!=SQLITE_OK){
        return false ;
    }
    sqlite3_bind_int(stmt1,1,product_id);
    //sqlite3_bind_int(stmt1,2,user_id);
    if (sqlite3_step(stmt1)!=SQLITE_ROW){
        return false;
    }
    stock_count=sqlite3_column_int(stmt1,0);
    if (quantity>stock_count){
        return false;
    }
    sqlite3_finalize(stmt1);

    

    sqlite3_stmt *stmt;
    const char *sql = "UPDATE cart SET quantity = ? WHERE product_id = ?;";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, quantity);
    sqlite3_bind_int(stmt, 2, product_id);
    
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

bool remove_from_cart(int product_id, int user_id) {
    if (!db) return false;

    sqlite3_stmt *stmt;
    const char *sql = "DELETE FROM cart WHERE product_id = ? AND user_id=?;";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, product_id);
    sqlite3_bind_int(stmt,2,user_id);
    
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}




bool cart_to_order( Order *order, int user_id){
    if(!db||!order){
        return false;

    }
    sqlite3_stmt* stmt;
    
    float total_price1=0;

    const char *sql="SELECT product_id , quantity FROM cart WHERE user_id=?;";
    if (sqlite3_prepare_v2(db,sql,-1,&stmt,NULL)!=SQLITE_OK){
        return false;
    }
    sqlite3_bind_int(stmt,1,user_id);

    while(sqlite3_step(stmt)==SQLITE_ROW){
        int product_id1=sqlite3_column_int(stmt,0);
        int count1=sqlite3_column_int(stmt,1);
    
        total_price1+=count1*total_price(product_id1);

    }
    sqlite3_finalize(stmt);
    order->total_amount=total_price1;
    return true;

    
}




bool get_user_cart(int user_id, Cart **cart_items, int *count){
  if (!db||!count){
       return false;
  }
  *count =0;
  sqlite3_stmt *stmt ;
  const char *sql="SELECT * FROM cart WHERE user_id=?;";
  if (sqlite3_prepare_v2(db,sql,-1,&stmt,NULL)!=SQLITE_OK){
    return false;
  }
  sqlite3_bind_int(stmt,1,user_id);
  while(sqlite3_step(stmt)==SQLITE_ROW){
    (*count)++;
  }
  sqlite3_reset(stmt);
  *cart_items= (Cart *)malloc(sizeof(Cart)*(*count));
  if (!*cart_items){
    sqlite3_finalize(stmt);
    return false;
  }

  int i =0;
  while(sqlite3_step(stmt)==SQLITE_ROW){
    Cart *cart = &(*cart_items)[i++];
     cart-> cart_id=sqlite3_column_int(stmt,0);
    printf("cart id is  %d\n",cart->cart_id);
     cart->user_id=sqlite3_column_int(stmt,1);
    printf("user id is  %d\n",cart->user_id);
     cart->product_id=sqlite3_column_int(stmt,2);
    printf("product id is  %d\n",cart->product_id);
     cart->quantity=sqlite3_column_int(stmt,3);
    printf("quantity is  %d\n",cart->quantity);
     cart->added_at=sqlite3_column_int(stmt,4);
  }
  sqlite3_finalize(stmt);
  return true;
}

bool remove_item_from_cart(int product_id, int user_id){                                //done
 if (!db){
    return false;
 }

 sqlite3_stmt *stmt;
 const char *sql ="DELETE FROM cart WHERE product_id=? AND user_id=?;";
 if (sqlite3_prepare_v2(db,sql,-1,&stmt,NULL)!=SQLITE_OK){
    return false;
 }
 sqlite3_bind_int(stmt,1,product_id);
 sqlite3_bind_int(stmt,2,user_id);

 bool success=sqlite3_step(stmt)==SQLITE_DONE;
 sqlite3_finalize(stmt);
 return success;
}


bool add_to_cart(const Cart *cart){                               //done
    if (!db){
        return false;
    }
    sqlite3_stmt *stmt1;
    const char* sql1= "SELECT stock FROM products WHERE product_id =?;";
    int stock_count;
    if(sqlite3_prepare_v2(db , sql1,-1,&stmt1,NULL)!=SQLITE_OK){
        printf("Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return false ;
    }
    sqlite3_bind_int(stmt1,1,cart->product_id);
    if (sqlite3_step(stmt1)!=SQLITE_ROW){
        printf("Failed to execute statement:\n");
        return false;
    }
    stock_count=sqlite3_column_int(stmt1,0);
    if (cart->quantity>stock_count){
        printf("quantity is too much\n");
        return false;
    }
    sqlite3_finalize(stmt1);


    sqlite3_stmt *stmt;
    const char* sql="INSERT INTO cart(user_id,product_id,quantity,last_modified) VALUES(?,?,?,?);";
    if (sqlite3_prepare_v2(db , sql,-1,&stmt,NULL)!=SQLITE_OK){
        printf("Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return false;
    }
    sqlite3_bind_int(stmt,1, cart->user_id);
    sqlite3_bind_int(stmt,2, cart->product_id);
    sqlite3_bind_int(stmt,3, cart->quantity);
    sqlite3_bind_int(stmt,4, cart->added_at);

    bool success =sqlite3_step(stmt);

    sqlite3_finalize(stmt);
    return success;
}

bool edit_cart_delete_cart(int quantity ,int user_id,int product_id){                    //done
    sqlite3_stmt *stmt ;
    const char * sql="UPDATE products SET stock= stock - ? WHERE product_id =? ;";
    if(sqlite3_prepare_v2(db,sql,-1,&stmt,NULL)!=SQLITE_OK){
        return false;
    }
    sqlite3_bind_int(stmt,1,quantity);
    sqlite3_bind_int(stmt,2,product_id);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);


    sqlite3_stmt *stmt1;
    const char *sql1="DELETE FROM cart WHERE user_id=?;";
    if (sqlite3_prepare_v2(db,sql1,-1,&stmt1,NULL)!=SQLITE_OK){
        return false;
    }
    sqlite3_bind_int(stmt1,1,user_id);
    sqlite3_step(stmt1);
    sqlite3_finalize(stmt1);
    return true;
}

