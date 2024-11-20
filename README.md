# CS3101-E-Commerce-App-in-C
Below is the main DBMS ER-diagram behind our backend(2F normalized when considering scalability)
```mermaid
erDiagram
    USERS ||--o{ ADDRESSES : "has"
    USERS ||--o{ CART : "adds"
    USERS ||--o{ ORDERS : "places"
    USERS ||--o{ WISHLISTS : "creates"
    USERS ||--o{ REVIEWS : "writes"
    USERS ||--o{ NOTIFICATIONS : "receives"
    USERS ||--o{ PRODUCT_SEARCH_LOG : "logs"

    PRODUCTS ||--o{ CART : "can be in"
    PRODUCTS ||--o{ ORDER_ITEMS : "included in"
    PRODUCTS ||--o{ WISHLISTS : "can be wishlisted"
    PRODUCTS ||--o{ REVIEWS : "has"
    PRODUCTS }o--|| CATEGORIES : "belongs to"

    ORDERS ||--o{ ORDER_ITEMS : "contains"
    ORDERS ||--o{ PAYMENTS : "has"
    ORDERS ||--o{ SHIPPING : "tracks"
    ORDERS ||--o{ BILLS : "generates"

    USERS {
        int user_id PK
        string username
        string password_hash
        string email
        string phone
        int address_default
        timestamp created_at
        timestamp last_login
        int status
        boolean email_verified
        boolean phone_verified
        blob profile_image
    }

    ADDRESSES {
        int address_id PK
        int user_id FK
        string address_line
        string city
        string postal_code
        string country
        string type
    }

    PRODUCTS {
        int product_id PK
        string name
        string description
        decimal price
        int stock
        string sku
        decimal discount
        decimal rating
        int category_id
        blob product_image
        timestamp created_at
        timestamp updated_at
        boolean is_active
    }

    CATEGORIES {
        int category_id PK
        string name
        string description
        int parent_id
    }

    CART {
        int cart_id PK
        int user_id FK
        int product_id FK
        int quantity
        timestamp last_modified
    }

    ORDERS {
        int order_id PK
        int user_id FK
        int status
        int payment_status
        int delivery_type
        int address_id FK
        time delivery_time
        timestamp estimated_delivery_date
        int order_status
        decimal total_amount
        timestamp created_at
        timestamp updated_at
        int bill_status
        boolean is_delayed
    }

    ORDER_ITEMS {
        int order_item_id PK
        int order_id FK
        int product_id FK
        int quantity
        decimal price
        decimal subtotal
    }

    PAYMENTS {
        int payment_id PK
        int order_id FK
        decimal amount
        int method
        int status
        string transaction_id
        timestamp payment_date
    }

    WISHLISTS {
        int wishlist_id PK
        int user_id FK
        int product_id FK
        timestamp added_at
    }

    REVIEWS {
        int review_id PK
        int user_id FK
        int product_id FK
        int rating
        string comment
        timestamp created_at
    }

    SHIPPING {
        int shipping_id PK
        int order_id FK
        string carrier_name
        string tracking_number
        string status
        timestamp shipped_at
        timestamp delivered_at
    }

    COUPONS {
        int coupon_id PK
        int code
        string discount_type
        decimal discount_value
        decimal min_order_value
        timestamp start_date
        timestamp end_date
        boolean is_active
        int usage_limit
        int usage_count
        timestamp created_at
    }

    BILLS {
        int bill_id PK
        int order_id FK
        int user_id FK
        decimal amount
        int status
        timestamp bill_date
    }

    NOTIFICATIONS {
        int notification_id PK
        int user_id FK
        string message
        boolean is_read
        timestamp created_at
    }

    DELIVERY_VANS {
        int van_id PK
        boolean is_available
        timestamp next_available
        int capacity
        string current_location
    }

    ADMINS {
        string admin_id PK
        string admin_name
        string admin_password_hash
    }

    PRODUCT_SEARCH_LOG {
        int search_id PK
        int user_id FK
        string search_term
        int results_count
        timestamp timestamp
    }
```


## Authors: Shuvam Banerji Seal(22MS076)  and Soumya Ghorai(22MS005)

This is our submission for the project work given by [Dr. Kripabandhu Ghosh](https://orcid.org/0000-0002-8130-1221).

### Below are a brief breakdown of contributions to the major features:

|Feature|Main Contributer|Reason For Inclusion|
|:---:|:---:|:---:|
|DBMS using SQLite3| Both authors have equal contribution| Needed a more scalable and rigid backend|
|BK-Tree Algorithm| Soumya Ghorai| Native spelling rectifier to correct the search terms given by user during retrieval from the database|
|BM-25 Algorithm| Shuvam Banerji Seal | With parameters k1=0.6, b=1.0, found to be good for vague retrieval, can be used as a RAG-like chatbot given smaller chunk-like docs as provided are there|
|GTK4 with Adwaita|Shuvam Banerji Seal|Planned to make gui for the whole app, but due to several constraints, it became not plausible|
|TRIGGERs in SQLite3|Soumya Ghorai| To maintain uniquesness in PK IDs for the users and admins|

## How to Run?
```bash
gcc pkg-config --cflags gtk4 libadwaita-1 chatbot/chatbot-gtk-final.c -o masterchatbot pkg-config --libs gtk4 libadwaita-1 -lm
```
```bash
gcc main.c -o main -lm -lsqlite3       
```
Run the above GCC commands to create the executables.
