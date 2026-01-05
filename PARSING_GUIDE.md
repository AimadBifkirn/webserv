# NGINX Config Parsing Guide - Structure Breakdown

## What Your Parser Needs to Understand

### 1. LEXICAL LEVEL (Tokenization)
Your parser should break the config into tokens:
- **Identifiers**: `server`, `location`, `listen`, `root`, etc.
- **Values**: `8080`, `/var/www`, `example.com`, `on`, `off`
- **Operators**: `{`, `}`, `;`
- **Comments**: Everything after `#` to end of line

### 2. SYNTACTIC LEVEL (Grammar)

The grammar is simple:
```
config          → (statement)*
statement       → directive | block
directive       → KEY VALUE* ';'
block           → KEY VALUE* '{' (statement)* '}'
```

Example parsing tree:
```
server {                           <- Block named "server"
    listen 127.0.0.1:8080;        <- Directive: key="listen", values=["127.0.0.1:8080"]
    server_name example.com;       <- Directive: key="server_name", values=["example.com"]
    
    location / {                   <- Block named "location", args="/"
        root /var/www/html;        <- Directive: key="root", values=["/var/www/html"]
        methods GET POST;          <- Directive: key="methods", values=["GET", "POST"]
    }
}
```

### 3. SEMANTIC LEVEL (Meaning)

After parsing tokens, create data structures:

#### Server Block
```cpp
struct ServerConfig {
    std::string listen;              // "127.0.0.1:8080"
    std::vector<std::string> server_names;  // ["example.com", "www.example.com"]
    std::string client_max_body_size; // "10M"
    std::map<int, std::string> error_pages; // {404: "/404.html", 500: "/50x.html"}
    std::vector<LocationConfig> locations;
};
```

#### Location Block
```cpp
struct LocationConfig {
    std::string path;                // "/"
    std::string root;                // "/var/www/html"
    std::vector<std::string> indices; // ["index.html", "index.htm"]
    std::vector<std::string> methods; // ["GET", "POST"]
    bool autoindex;                  // true or false
    std::string return_code;         // "301 /new"
    bool cgi_enabled;
    std::string cgi_extension;
    std::string cgi_path;
    bool upload_enabled;
    std::string upload_path;
};
```

---

## PARSING ALGORITHM

### Step 1: Read and Tokenize
```
Input: "server { listen 8080; }"
↓
Tokens: ["server", "{", "listen", "8080", ";", "}"]
```

### Step 2: Parse into AST (Abstract Syntax Tree)
```
Parse tokens recursively:
- If token is identifier and next is '{':  Parse block
- If token is identifier and next is value(s): Parse directive
- If token is ';': End of directive
- If token is '}': End of block
- If token is '#': Skip to end of line
```

### Step 3: Validate and Convert
```
- Check syntax correctness
- Validate values (ports are numbers, paths exist, etc.)
- Create config objects from AST
- Apply defaults for missing values
```

### Step 4: Use
```
Your server code uses the parsed config objects
to determine how to handle requests
```

---

## SIMPLE EXAMPLE: Minimal Config

### Input:
```nginx
server {
    listen 8080;
    location / {
        root ./www;
        methods GET;
    }
}
```

### After Parsing:
```
ServerConfig server1:
  listen = "0.0.0.0:8080"
  server_names = []
  
  LocationConfig loc1:
    path = "/"
    root = "./www"
    methods = ["GET"]
```

### What happens when request comes in:
1. Server receives HTTP request on port 8080
2. Looks up config for that server
3. Matches URL path "/" to location block
4. Serves files from "./www"
5. Only accepts GET requests

---

## DECISION POINTS FOR YOUR IMPLEMENTATION

### Should You Support?
1. **Multiple servers?** YES (most real configs have this)
2. **Multiple listen directives?** YES (one server can listen on multiple ports)
3. **Regex locations?** OPTIONAL (advanced, but nice to have)
4. **Nested location blocks?** OPTIONAL (complex but powerful)
5. **Comments?** YES (common in configs)
6. **Default values?** YES (simplifies configs)

### Parser Complexity Levels:

**Level 1 (Minimal):**
- Simple directives only
- One server block
- Basic error handling

**Level 2 (Standard):**
- Multiple servers
- Server + location blocks
- Most directives from the guide
- Comment support

**Level 3 (Advanced):**
- Regex locations
- Nested locations
- More complex directives
- Full error validation

---

## Common Pitfalls to Avoid

1. **Not handling whitespace correctly**
   ```
   listen    8080 ;    <- Extra spaces, should still parse
   ```

2. **Not supporting multiple values**
   ```
   methods GET POST DELETE;    <- Multiple values for one directive
   ```

3. **Not handling different path formats**
   ```
   root /var/www/html;         <- Absolute path
   root ./www;                 <- Relative path
   root ~/html;                <- Home directory
   ```

4. **Not validating numbers**
   ```
   listen abc;         <- Should error (port not a number)
   port -1;            <- Should error (invalid port range)
   ```

5. **Forgetting directive inheritance**
   ```
   server {
       root /var/www;          <- Default for all locations
       location / {
           root /var/www/html; <- Overrides server root
       }
   }
   ```

---

## Testing Your Parser

Create test configs:

### Test 1: Simple config
```nginx
server {
    listen 8080;
    location / {
        root ./www;
    }
}
```

### Test 2: Multiple servers
```nginx
server {
    listen 8080;
}
server {
    listen 9090;
}
```

### Test 3: Multiple directives
```nginx
server {
    listen 8080;
    server_name example.com www.example.com;
    
    location / {
        root /var/www;
        methods GET POST DELETE;
    }
}
```

### Test 4: Error cases
```nginx
server {
    listen invalid;         <- Non-numeric port
    location / {
        root /nonexistent;  <- Path doesn't exist
    }
}
```

---

## Next Steps

1. Design your parser classes/functions
2. Implement tokenizer (lexer)
3. Implement parser (syntax tree builder)
4. Implement validator (semantic checker)
5. Create config objects from parsed data
6. Test with various config files
