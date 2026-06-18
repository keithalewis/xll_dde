# Testing DDE Advise Loop with Excel

## How to Test the DDE Server

1. **Run the DDE Server** (`xll_dde.exe`)
   - You'll see: "DDE Server started: MyService|MyTopic"
   - Updates will be posted every 3 seconds

2. **In Excel**, use one of these formulas:

   **Hot Link (Advise Loop - Auto Updates):**
   ```
   =MyService|MyTopic!MyItem
   ```
   - This establishes an advise loop
   - Excel will automatically receive updates when data changes
   - You'll see the server console show:
     - "AdvStart requested for item: MyItem"
     - Every 3 seconds: "AdvReq for item: MyItem - Sending: Update #X..."

   **Cold Link (One-Time Request):**
   ```
   =REQUEST("MyService","MyTopic","MyItem")
   ```
   - This requests data once
   - No automatic updates

## What Happens Under the Hood

### When Excel Opens a Hot Link:
1. Excel sends `XTYP_CONNECT` → Server responds TRUE
2. Excel sends `XTYP_ADVSTART` → Server's `handleAdvStart` is called → Returns TRUE
3. Excel sends `XTYP_ADVREQ` → Server's `handleAdvReq` provides initial data

### When Server Updates Data:
1. Timer fires every 3 seconds
2. `currentData` is updated
3. `service.PostAdvise("MyTopic", "MyItem")` is called
4. All clients with active advise loops receive notification
5. Excel sends `XTYP_ADVREQ` → Server's `handleAdvReq` provides new data
6. Excel cell automatically updates!

## Server Console Output Example

```
DDE Server started: MyService|MyTopic
Updates will be posted every 3 seconds...
Try connecting with Excel: =MyService|MyTopic!MyItem

AdvStart requested for item: MyItem (Format: 13)
AdvReq for item: MyItem - Sending: Initial value

[Timer] Data updated: Update #1 at 14:23:15
[Timer] Posted advise notification
AdvReq for item: MyItem - Sending: Update #1 at 14:23:15

[Timer] Data updated: Update #2 at 14:23:18
[Timer] Posted advise notification
AdvReq for item: MyItem - Sending: Update #2 at 14:23:18
```

## Manual Triggering

You can also trigger updates manually instead of using a timer:

```cpp
// Update your data
currentData = TEXT("New value");

// Notify all clients
service.PostAdvise(TEXT("MyTopic"), TEXT("MyItem"));
```

## Multiple Items

To support multiple items, use different item names:

```cpp
// Notify specific item
service.PostAdvise(TEXT("MyTopic"), TEXT("Price"));

// Notify ALL items in topic (pass empty string)
service.PostAdvise(TEXT("MyTopic"), TEXT(""));
```

## Advanced: Track Active Advise Loops

If you need to track which clients have active advise loops:

```cpp
std::map<HSZ, std::set<HCONV>> activeAdviseLoops;

th.handleAdvStart = [&](UINT uFmt, HCONV hConv, HSZ item, HSZ topic) -> BOOL {
    activeAdviseLoops[item].insert(hConv);
    return TRUE;
};

th.handleDisconnect = [&](HCONV hConv) {
    // Remove from all items
    for (auto& pair : activeAdviseLoops) {
        pair.second.erase(hConv);
    }
};
```
