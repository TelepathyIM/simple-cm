## Engineering CM

The Engineering manager let you interactively play on the service side:
- Customize manager and protocol names (there are presets for popular CMs)
- Manually control the contact list (add contacts and adjust their presence)
- Send a message on behalf of a contact (plain text or JSON format)
- Receive (display) a message from Telepathy Client

## JSON Message Format

#### Plain text message
```
[
    {
        "message-received": 1588463144,
        "message-sender-id": "contact1",
        "message-sent": 1588463144,
        "message-token": "001",
        "message-type": 0
    },
    {
        "content": "Hello from JSON",
        "content-type": "text/plain"
    }
]

```

#### Message with a linked WebPage
```
[
    {
        "message-received": 1588463145,
        "message-sender-id": "contact1",
        "message-sent": 1588463145,
        "message-token": "002",
        "message-type": 0
    },
    {
        "interface": "org.freedesktop.Telepathy.Channel.Interface.WebPage",
        "url": "https://www.qt.io/",
        "siteName": "www.qt.io",
        "title": "Qt | Cross-platform software development",
        "description": "Qt is the faster, smarter way to ..."
    },
    {
        "content": "https://www.qt.io/",
        "content-type": "text/plain"
    }
]
```

#### Forwarded Message
```
[
    {
        "message-received": 1588463146,
        "message-sender-id": "contact1",
        "message-sent": 1588463146,
        "message-token": "003",
        "message-type": 0
    },
    {
        "interface": "org.freedesktop.Telepathy.Channel.Interface.Forwarding",
        "message-sender-id": "contact2",
        "message-sender-alias": "Contact 2",
        "message-sent": 1588463008
    },
    {
        "content": "Forwarded content (the only displayed part if the iface not supported)",
        "content-type": "text/plain"
    }
]
```
