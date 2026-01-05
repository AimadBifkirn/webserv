# NGINX Config File - Complete Elements Guide

## Overview
An NGINX config file is hierarchical with three main levels:
1. **Global level** - Basic directives
2. **Server block level** - `server { }` - defines virtual hosts
3. **Location block level** - `location { }` - defines route-specific rules

---

## Top-Level Structure

```
directive value;              # Simple directive (ends with ;)
block_name {                  # Block directive (uses {})
    directive value;
    nested_block {
        directive value;
    }
}
```

---

## SERVER BLOCK DIRECTIVES

### Listen Directive
**Purpose:** Specifies the IP address and port the server listens on

```nginx
listen 8080;                    # Port only (0.0.0.0:8080)
listen 127.0.0.1:8080;        # IP:Port
listen 0.0.0.0:8080;          # All interfaces
listen [::1]:8080;            # IPv6
```

**What to parse:**
- IP address (optional, defaults to 0.0.0.0)
- Port number (required)
- Can have multiple `listen` directives

---

### Server Name Directive
**Purpose:** Virtual host names - matches HTTP `Host` header

```nginx
server_name example.com;
server_name example.com www.example.com;    # Multiple names
server_name *.example.com;                   # Wildcard
server_name ~^example\.com$;                 # Regex (advanced)
server_name _;                               # Default/catch-all
```

**What to parse:**
- One or more domain names
- Wildcards and regex patterns (optional complexity)
- If missing, matches any request

---

### Client Max Body Size
**Purpose:** Maximum request body size (file uploads)

```nginx
client_max_body_size 10M;      # 10 megabytes
client_max_body_size 1024;     # 1024 bytes
client_max_body_size 1G;       # Gigabytes
```

**What to parse:**
- Number + optional unit (B, K, M, G)
- Default usually 1M

---

### Error Page Directive
**Purpose:** Custom error page paths

```nginx
error_page 404 /404.html;
error_page 500 502 503 504 /50x.html;    # Multiple error codes
error_page 404 /errors/not-found.html;
```

**What to parse:**
- One or more HTTP status codes
- Path to error page file

---

## LOCATION BLOCK DIRECTIVES

Location blocks define rules for specific URL paths:

```nginx
location / {                    # Root path
    ...
}

location /api {                 # Exact path
    ...
}

location /files/ {              # Trailing slash matters
    ...
}

location ~ \.php$ {             # Regex (case-sensitive)
    ...
}

location ~* \.jpg$ {            # Regex (case-insensitive)
    ...
}

location ^~ /admin {            # Prefix match (no regex after)
    ...
}
```

---

### Root Directive
**Purpose:** Base directory for serving files

```nginx
root /var/www/html;
root ./public;                  # Relative path
root /home/user/website;
```

**What to parse:**
- Absolute or relative file path
- File path should exist (you might want to validate)

---

### Index Directive
**Purpose:** Default files to serve when requesting a directory

```nginx
index index.html;
index index.html index.htm default.html;    # Try in order
```

**What to parse:**
- One or more file names
- Server tries them in order, serves first match

---

### Methods Directive
**Purpose:** Allowed HTTP methods for this location

```nginx
methods GET;
methods GET POST;
methods GET POST DELETE PUT HEAD;
```

**What to parse:**
- One or more HTTP method names
- Common methods: GET, POST, PUT, DELETE, HEAD, OPTIONS
- If a method isn't allowed, return 405 Method Not Allowed

---

### Autoindex Directive
**Purpose:** Enable/disable directory listing (showing folder contents)

```nginx
autoindex on;                   # Show directory listing
autoindex off;                  # Don't list directories
```

**What to parse:**
- Boolean value: `on` or `off`

---

### Return Directive
**Purpose:** HTTP redirects and custom responses

```nginx
return 301 /new-path;           # Permanent redirect
return 302 /temporary;          # Temporary redirect
return 404;                     # Not found (no redirect)
return 403;                     # Forbidden
```

**What to parse:**
- HTTP status code
- Optional redirect URL

---

### CGI Directives (Advanced)
**Purpose:** Execute scripts on the server

```nginx
cgi on;                         # Enable CGI
cgi_extension .php;             # File extension to trigger CGI
cgi_path /usr/bin/php-cgi;     # Path to interpreter
```

**What to parse:**
- Boolean for `cgi on/off`
- File extension (e.g., .php, .py)
- Path to interpreter executable

---

### Upload Directives
**Purpose:** Handle file uploads

```nginx
upload on;                      # Enable file upload handling
upload_path /var/www/uploads;  # Directory to store uploads
```

**What to parse:**
- Boolean for `upload on/off`
- Directory path for stored files

---

## PARSING RULES YOU MUST FOLLOW

### 1. **Syntax Rules**
- Directives end with semicolon (`;`)
- Block directives use curly braces `{ }`
- Comments start with `#` and go to end of line
- Whitespace (spaces, tabs, newlines) is flexible

### 2. **Hierarchy Rules**
```
Global directives
├── server block
│   ├── server-level directives
│   └── location block
│       └── location-level directives
└── server block
    └── ...
```

### 3. **Scope Rules**
- Some directives can only be in `server` blocks
- Some only in `location` blocks
- Some can be in both (location overrides server)

### 4. **Error Handling**
- If config has syntax errors, reject/warn
- If required directives missing, use defaults
- If file paths don't exist, warn but continue

### 5. **Defaults**
```nginx
listen:                  0.0.0.0:80
server_name:            (matches any)
root:                   ./
index:                  index.html
methods:                GET POST (usually)
autoindex:              off
client_max_body_size:   1M
```

---

## EXAMPLE: MINIMAL CONFIG

```nginx
server {
    listen 8080;
    server_name localhost;
    
    location / {
        root ./www;
        index index.html;
        methods GET POST;
    }
}
```

---

## EXAMPLE: FULL-FEATURED CONFIG

```nginx
server {
    listen 127.0.0.1:8080;
    server_name example.com www.example.com;
    
    client_max_body_size 10M;
    error_page 404 /404.html;
    
    # Static files
    location / {
        root /var/www/html;
        index index.html;
        methods GET POST;
        autoindex off;
    }
    
    # Public directory with listing
    location /public {
        root /var/www;
        methods GET;
        autoindex on;
    }
    
    # Upload endpoint
    location /upload {
        methods POST;
        upload on;
        upload_path /var/www/uploads;
    }
    
    # CGI scripts
    location /cgi-bin {
        methods GET POST;
        cgi on;
        cgi_extension .php;
        cgi_path /usr/bin/php-cgi;
    }
    
    # Redirect
    location /old {
        return 301 /new;
    }
}
```

---

## PARSING CHECKLIST

When you write your parser, ensure it handles:

- [x] Multiple `server` blocks
- [x] Multiple `location` blocks per server
- [x] Simple directives with values
- [x] Block directives with nested content
- [x] Comments (ignore them)
- [x] Flexible whitespace
- [x] Proper error reporting
- [x] Default values
- [x] Nested location blocks (if supporting)
- [x] Directive inheritance (location overrides server)
