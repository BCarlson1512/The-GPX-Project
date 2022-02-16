/*
* Name: Ben Carlson
* Student ID: 1044277
* Code for parser based on files libXmlExample.c, StructListDemo.c sourced from CIS*2750 courselink
* Provided by Denis Nikitenko
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "GPXParser.h"
#include <stdbool.h>

//simple xml parser from a provided filename
xmlDoc *createDoc(char *filename)
{
    if (filename == NULL || filename[0] == '\0')
    {
        return NULL;
    }
    xmlDoc *doc = xmlReadFile(filename, NULL, 0);
    if (doc == NULL)
    {
        xmlCleanupParser();
        return NULL;
    }
    return doc;
}

// converts an attribute and its content into our gpx data
void headerToData(GPXdoc *gpxObj, xmlNode *node)
{
    strcpy(gpxObj->namespace, (char *)node->ns->href);
    xmlAttr *attr;
    for (attr = node->properties; attr != NULL; attr = attr->next)
    {
        if (!strcmp((char *)attr->name, "version"))
        {
            gpxObj->version = atof((char *)attr->children->content);
        }
        if (!strcmp((char *)attr->name, "creator"))
        {
            gpxObj->creator = malloc(sizeof(char) * (strlen((char *)attr->children->content) + 1));
            strcpy(gpxObj->creator, (char *)attr->children->content);
        }
    }
}

// takes the xml node tagged as "wpt" and creates a waypoint object out of it
// when GPX data is encoutered, create objects for the gpx data
Waypoint *wptToData(xmlNode *node)
{
    Waypoint *newWpt = malloc(sizeof(Waypoint));
    newWpt->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
    newWpt->name = calloc(1, sizeof(char));
    xmlNode *child;
    for (child = node->children; child != NULL; child = child->next)
    {
        if (child->type == XML_ELEMENT_NODE)
        {
            //printf("Name: %s \n", child->name);
            if (!strcmp("name", (char *)child->name))
            {
                int size = strlen((char *)child->children->content) + 1;
                newWpt->name = realloc(newWpt->name, sizeof(char) * size);
                strcpy(newWpt->name, (char *)child->children->content);
            }
            else
            {
                GPXData *newGPXData = malloc(sizeof(GPXData) + (sizeof(char *) * strlen((char *)child->children->content)));
                strcpy(newGPXData->name, (char *)child->name);
                strcpy(newGPXData->value, (char *)child->children->content);
                insertBack(newWpt->otherData, newGPXData);
            }
        }
    }
    xmlAttr *attr;
    for (attr = node->properties; attr != NULL; attr = attr->next)
    {
        xmlNode *value = attr->children;
        char *attrName = (char *)attr->name;
        char *content = (char *)(value->content);
        if (!strcmp(attrName, "lat"))
        {
            newWpt->latitude = atof(content);
        }
        else if (!strcmp(attrName, "lon"))
        {
            newWpt->longitude = atof(content);
        }
    }
    return newWpt;
}

// takes an xml node tagged as "rte" and creates an Route object out of it
// will pass nodes tagged as "rtept" to wptToData to process them as waypoints
Route *rteToData(xmlNode *node)
{
    Route *newRte = malloc(sizeof(Route));
    newRte->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    newRte->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
    newRte->name = calloc(1, sizeof(char));
    xmlNode *child;

    for (child = node->children; child != NULL; child = child->next)
    {
        if (child->type == XML_ELEMENT_NODE)
        {
            if (!strcmp("name", (char *)child->name))
            {
                int size = (strlen((char *)child->children->content) + 1);
                newRte->name = realloc(newRte->name, size * sizeof(char));
                strcpy(newRte->name, (char *)child->children->content);
            }
            else if (!strcmp("rtept", (char *)child->name))
            {
                Waypoint *rtePoint = wptToData(child);
                insertBack(newRte->waypoints, rtePoint);
            }
            else
            {
                GPXData *newGPXData = malloc(sizeof(GPXData) + (sizeof(char *) * strlen((char *)child->children->content)));
                strcpy(newGPXData->name, (char *)child->name);
                strcpy(newGPXData->value, (char *)child->children->content);
                insertBack(newRte->otherData, newGPXData);
            }
        }
    }
    return newRte;
}

// takes an xml node tagged as "trkseg" and creates a track segment object out of it
// passes nodes tagged as "trkpt" to wptToData to create waypoints
TrackSegment *segToData(xmlNode *node)
{
    TrackSegment *newTrkSeg = malloc(sizeof(TrackSegment));
    newTrkSeg->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    xmlNode *child;

    for (child = node->children; child != NULL; child = child->next)
    {
        if (child->type == XML_ELEMENT_NODE)
        {
            if (!strcmp("trkpt", (char *)child->name))
            {
                Waypoint *wpt = wptToData(child);
                insertBack(newTrkSeg->waypoints, wpt);
            }
        }
    }
    return newTrkSeg;
}

// takes xml nodes tagged "trk" and creates a track object
// passes nodes tagged as "trkseg" to segToData to create track segments
Track *trkToData(xmlNode *node)
{
    Track *newTrk = malloc(sizeof(Track));
    newTrk->name = calloc(1, sizeof(char *));
    newTrk->segments = initializeList(&trackSegmentToString, &deleteTrackSegment, &compareTrackSegments);
    newTrk->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
    xmlNode *child;
    for (child = node->children; child != NULL; child = child->next)
    {
        if (child->type == XML_ELEMENT_NODE)
        {
            if (!strcmp("name", (char *)child->name))
            {
                int size = sizeof(char) * strlen((char *)child->children->content) + 1;
                newTrk->name = realloc(newTrk->name, size);
                strcpy(newTrk->name, (char *)child->children->content);
            }
            else if (!strcmp("trkseg", (char *)child->name))
            {
                TrackSegment *trkSeg = segToData(child);
                insertBack(newTrk->segments, trkSeg);
            }
            else
            {
                GPXData *newGPXData = malloc(sizeof(GPXData) + (sizeof(char *) * strlen((char *)child->children->content)));
                strcpy(newGPXData->name, (char *)child->name);
                strcpy(newGPXData->value, (char *)child->children->content);
                insertBack(newTrk->otherData, newGPXData);
            }
        }
    }
    return newTrk;
}

// recursively reads data in from the gpx file
static void readData(GPXdoc *gpxObj, xmlNode *a_node)
{
    xmlNode *cur_node;
    for (cur_node = a_node; cur_node != NULL; cur_node = cur_node->next)
    {
        if (cur_node->type == XML_ELEMENT_NODE)
        {
            if (!strcmp((char *)cur_node->name, "gpx"))
            {
                headerToData(gpxObj, cur_node);
            }
            else if (!strcmp((char *)cur_node->name, "wpt"))
            {
                Waypoint *wpt = wptToData(cur_node);
                insertBack(gpxObj->waypoints, wpt);
            }
            else if (!strcmp((char *)cur_node->name, "rte"))
            {
                Route *rte = rteToData(cur_node);
                insertBack(gpxObj->routes, rte);
            }
            else if (!strcmp((char *)cur_node->name, "trk"))
            {
                Track *trk = trkToData(cur_node);
                insertBack(gpxObj->tracks, trk);
            }
        }
        readData(gpxObj, cur_node->children);
    }
}

// parses the gpx file and creates an object full of its data
GPXdoc *createGPXdoc(char *filename)
{
    GPXdoc *gpxObj = malloc(sizeof(GPXdoc));
    gpxObj->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    gpxObj->routes = initializeList(&routeToString, &deleteRoute, &compareRoutes);
    gpxObj->tracks = initializeList(&trackToString, &deleteTrack, &compareTracks);
    xmlDoc *doc = createDoc(filename);
    xmlNode *root_element = xmlDocGetRootElement(doc);
    readData(gpxObj, root_element);
    if (doc == NULL || root_element == NULL)
    {
        freeList(gpxObj->waypoints);
        freeList(gpxObj->routes);
        freeList(gpxObj->tracks);
        free(gpxObj);
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return NULL;
    }
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return gpxObj;
}

/* 
* validates two libxmlTrees
* Referenced from http://knol2share.blogspot.com/2009/05/validate-xml-against-xsd-in-c.html 
*/
bool validateLibXmlTree(char *filename, char *gpxSchemaFile)
{
    if (filename == NULL || gpxSchemaFile == NULL)
    {
        return false;
    }
    // this block of code generates trees for the xmldoc as well as the xmlschemadoc
    xmlDocPtr doc;
    xmlSchemaPtr schemaDoc = NULL;
    xmlSchemaParserCtxtPtr ctxt;

    xmlLineNumbersDefault(1);

    ctxt = xmlSchemaNewParserCtxt(gpxSchemaFile);
    xmlSchemaSetParserErrors(ctxt, (xmlSchemaValidityErrorFunc)fprintf, (xmlSchemaValidityWarningFunc)fprintf, stderr);
    schemaDoc = xmlSchemaParse(ctxt);
    // for debugging
    // xmlSchemaDump(stdout, schema);  // prints schema to stdout

    doc = xmlReadFile(filename, NULL, 0);

    if (doc == NULL)
    { // parse error
        return false;
    }
    xmlSchemaValidCtxtPtr ctxtPtr;
    int validation;
    ctxtPtr = xmlSchemaNewValidCtxt(schemaDoc);
    xmlSchemaSetValidErrors(ctxtPtr, (xmlSchemaValidityErrorFunc)fprintf, (xmlSchemaValidityWarningFunc)fprintf, stderr);

    validation = xmlSchemaValidateDoc(ctxtPtr, doc);

    if (schemaDoc != NULL)
    { // free schema contents
        xmlSchemaFree(schemaDoc);
        xmlSchemaCleanupTypes();
        xmlCleanupParser();
        xmlMemoryDump();
    }

    xmlSchemaFreeValidCtxt(ctxtPtr);
    xmlFreeDoc(doc);

    if (validation == 0)
    {
        return true;
    }
    return false;
}

bool validateXMLTree(xmlDocPtr doc, char *gpxSchemaFile)
{ // validates an xmlDoc pointer against the gpx schema
    if (doc == NULL || gpxSchemaFile == NULL)
    { // parse error
        return false;
    }
    // this block of code generates trees for the xmldoc as well as the xmlschemadoc
    xmlSchemaPtr schemaDoc = NULL;
    xmlSchemaParserCtxtPtr ctxt;

    xmlLineNumbersDefault(1);

    ctxt = xmlSchemaNewParserCtxt(gpxSchemaFile);
    xmlSchemaSetParserErrors(ctxt, (xmlSchemaValidityErrorFunc)fprintf, (xmlSchemaValidityWarningFunc)fprintf, stderr);
    schemaDoc = xmlSchemaParse(ctxt);
    // for debugging
    // xmlSchemaDump(stdout, schema);  // prints schema to stdout

    xmlSchemaValidCtxtPtr ctxtPtr;
    int validation;
    ctxtPtr = xmlSchemaNewValidCtxt(schemaDoc);
    xmlSchemaSetValidErrors(ctxtPtr, (xmlSchemaValidityErrorFunc)fprintf, (xmlSchemaValidityWarningFunc)fprintf, stderr);

    validation = xmlSchemaValidateDoc(ctxtPtr, doc);

    if (schemaDoc != NULL)
    { // free schema contents
        xmlSchemaFree(schemaDoc);
        xmlSchemaCleanupTypes();
        xmlCleanupParser();
        xmlMemoryDump();
    }
    xmlSchemaFreeParserCtxt(ctxt);
    xmlSchemaFreeValidCtxt(ctxtPtr);

    if (validation == 0)
    {
        return true;
    }
    return false;
}

void wptToXMLDoc(Waypoint *wpt, xmlNodePtr root_node, char *type)
{ // converts a gpx waypoint to an xmldoc element
    if (wpt == NULL || root_node == NULL)
    {
        return;
    }
    xmlNodePtr wptNode = NULL;
    char buffer[1000];

    wptNode = xmlNewChild(root_node, NULL, BAD_CAST type, NULL);
    sprintf(buffer, "%f", wpt->latitude);
    xmlNewProp(wptNode, BAD_CAST "lat", BAD_CAST buffer);
    sprintf(buffer, "%f", wpt->longitude);
    xmlNewProp(wptNode, BAD_CAST "lon", BAD_CAST buffer);
    if (wpt->name[0] != '\0')
    {
        xmlNewChild(wptNode, NULL, BAD_CAST "name", BAD_CAST wpt->name);
    }
    ListIterator dataIter = createIterator(wpt->otherData);
    GPXData *data;
    while ((data = nextElement(&dataIter)) != NULL)
    { // reads any additional data into the waypoint
        xmlNewChild(wptNode, NULL, BAD_CAST data->name, BAD_CAST data->value);
    }
}

void rteToXMLDoc(Route *rte, xmlNodePtr root_node)
{ // Converts route to xml tree
    if (rte == NULL || root_node == NULL)
    {
        return;
    }
    xmlNodePtr rteNode = NULL;
    rteNode = xmlNewChild(root_node, NULL, BAD_CAST "rte", NULL);
    xmlNewChild(rteNode, NULL, BAD_CAST "name", BAD_CAST rte->name);
    ListIterator wptIter = createIterator(rte->waypoints), dataIter = createIterator(rte->otherData);
    Waypoint *wpt = NULL;
    GPXData *data = NULL;
    while ((data = nextElement(&dataIter)) != NULL)
    { // reads any additional data into the waypoint
        xmlNewChild(rteNode, NULL, BAD_CAST data->name, BAD_CAST data->value);
    }
    while ((wpt = nextElement(&wptIter)) != NULL)
    {
        wptToXMLDoc(wpt, rteNode, "rtept");
    }
}

void trkToXMLDoc(Track *trk, xmlNodePtr root_node)
{ // converts a track to an xml doc
    if (trk == NULL || root_node == NULL)
    {
        return;
    }
    ListIterator segIter = createIterator(trk->segments), dataIter = createIterator(trk->otherData), wptIter;
    TrackSegment *seg;
    Waypoint *wpt;
    GPXData *data;
    xmlNodePtr trkNode = NULL, segNode = NULL;
    trkNode = xmlNewChild(root_node, NULL, BAD_CAST "trk", NULL);
    xmlNewChild(trkNode, NULL, BAD_CAST "name", BAD_CAST trk->name);
    while ((data = nextElement(&dataIter)) != NULL)
    { // reads any additional data into the waypoint
        xmlNewChild(trkNode, NULL, BAD_CAST data->name, BAD_CAST data->value);
    }
    while ((seg = nextElement(&segIter)) != NULL)
    {
        wptIter = createIterator(seg->waypoints);
        segNode = xmlNewChild(trkNode, NULL, BAD_CAST "trkseg", NULL);
        while ((wpt = nextElement(&wptIter)) != NULL)
        {
            wptToXMLDoc(wpt, segNode, "trkpt");
        }
    }
}

bool verifyLat(float srclat, float destlat)
{ // checks if the latitude is within its proper range
    if (srclat < -90.0 || destlat < -90.0 || srclat > 90.0 || destlat > 90.0)
    {
        return false;
    }
    return true;
}

bool verifyLong(float srclong, float destlong)
{ // check if the longitude is within its proper range
    if (srclong < -180.0 || destlong < -180.0 || srclong > 180.0 || destlong > 180.0)
    {
        return false;
    }
    return true;
}

xmlDocPtr gpxDocToXMLDoc(GPXdoc *doc)
{ // converts the gpx struct to an xmlDoc
    xmlDocPtr tree = NULL;
    xmlNodePtr root_node = NULL;
    char buffer[1000];

    // creates the gpx tag of the document
    tree = xmlNewDoc(BAD_CAST "1.0");
    root_node = xmlNewNode(NULL, BAD_CAST "gpx");
    sprintf(buffer, "%.1f", doc->version);
    xmlNewProp(root_node, BAD_CAST "version", BAD_CAST buffer);
    sprintf(buffer, "%s", doc->creator);
    xmlNewProp(root_node, BAD_CAST "creator", BAD_CAST buffer);
    xmlNsPtr ns = xmlNewNs(root_node, BAD_CAST doc->namespace, NULL);
    xmlSetNs(root_node, ns);
    xmlDocSetRootElement(tree, root_node);
    // add child nodes for each waypoint
    ListIterator wptIter = createIterator(doc->waypoints);
    Waypoint *wp;

    while ((wp = nextElement(&wptIter)) != NULL)
    {
        wptToXMLDoc(wp, root_node, "wpt");
    }
    // add child nodes for each route
    ListIterator rteIter = createIterator(doc->routes);
    Route *rte;
    while ((rte = nextElement(&rteIter)) != NULL)
    {
        rteToXMLDoc(rte, root_node);
    }
    // add child nodes for each track
    ListIterator trkIter = createIterator(doc->tracks);
    Track *trk;
    while ((trk = nextElement(&trkIter)) != NULL)
    {
        trkToXMLDoc(trk, root_node);
    }
    xmlCleanupParser();
    return tree;
}

bool validateGPXData(GPXData *data)
{
    if (data->name == NULL || data->name == NULL || data->name[0] == '\0' || data->value == NULL || data->value[0] == '\0')
    { // if the data elements are not initialized...
        return false;
    }
    return true;
}

bool validateDocAttributes(GPXdoc *doc)
{ // this function checks every attribute stored in the GPXDoc (Namespace, creator, version)
    if (doc->namespace == NULL || doc->version <= 0.0)
    { // if the basic attributes are null or empty
        return false;
    }
    if (doc->waypoints == NULL || doc->routes == NULL || doc->tracks == NULL)
    { // if any list is uninitialized
        return false;
    }
    return true;
}

bool validateWaypoint(Waypoint *wpt)
{ // validates each individual waypoint
    if (wpt == NULL)
    { // if the waypoint straight up doesnt exist...
        return false;
    }
    if (!verifyLat(wpt->latitude, 0.0) || !verifyLong(wpt->longitude, 0.0))
    { // if the base attributes are uninitialized
        return false;
    }
    ListIterator dataIt = createIterator(wpt->otherData);
    GPXData *data;

    while ((data = nextElement(&dataIt)) != NULL)
    {
        if (!validateGPXData(data))
        {
            return false;
        }
    }

    return true;
}

bool validateWaypointLists(List *list)
{ // validates the list of waypoints
    if (list == NULL)
    {
        return false;
    }
    ListIterator li = createIterator(list);
    Waypoint *wpt;
    while ((wpt = nextElement(&li)) != NULL)
    {
        if (!validateWaypoint(wpt))
        {
            return false;
        }
    }
    return true;
}

bool validateRoute(Route *rte)
{ // validates each route
    if (rte == NULL)
    { // if the route doesnt exist...
        return false;
    }
    if (rte->name == NULL)
    { // if the name is not intialized
        return false;
    }
    ListIterator dataIt = createIterator(rte->otherData);
    GPXData *data;
    if (!validateWaypointLists(rte->waypoints))
    { // check thelist of waypoints
        return false;
    }
    while ((data = nextElement(&dataIt)) != NULL)
    { // check the attributes
        if (!validateGPXData(data))
        {
            return false;
        }
    }
    return true;
}

bool validateSegment(TrackSegment *seg)
{ // validates a track segment
    if (seg == NULL)
    { // if the seg doesnt exist
        return false;
    }
    if (!validateWaypointLists(seg->waypoints))
    { // validate the waypoints
        return false;
    }
    return true;
}

bool validateTrack(Track *trk)
{ // validates a track
    if (trk == NULL)
    { // if the track doesnt exist
        return false;
    }
    if (trk->name == NULL)
    { // if the name is not initialized
        return false;
    }
    ListIterator segIter = createIterator(trk->segments);
    ListIterator dataIter = createIterator(trk->otherData);
    TrackSegment *seg;
    GPXData *data;
    while ((seg = nextElement(&segIter)) != NULL)
    { // validate the segments
        if (!validateSegment(seg))
        {
            return false;
        }
    }
    while ((data = nextElement(&dataIter)) != NULL)
    { // check the attributes
        if (!validateGPXData(data))
        {
            return false;
        }
    }
    return true;
}

bool validateGPXDoc(GPXdoc *doc, char *gpxSchemaFile)
{ // check the GPXDoc created by createGPXdoc against a gpx xsd file
    if (doc == NULL || gpxSchemaFile == NULL || gpxSchemaFile[0] == '\0')
    {
        return false;
    }
    if (!validateDocAttributes(doc) || !validateWaypointLists(doc->waypoints))
    {
        return false;
    }
    ListIterator rteIter = createIterator(doc->routes);
    ListIterator trkIter = createIterator(doc->tracks);
    Track *currTrk;
    Route *currRte;
    while ((currRte = nextElement(&rteIter)) != NULL)
    {
        if (!validateRoute(currRte))
        {
            return false;
        }
    }
    while ((currTrk = nextElement(&trkIter)) != NULL)
    {
        if (!validateTrack(currTrk))
        {
            return false;
        }
    }
    // now that we are sure the document is valid... create an xml tree out of the doc and check it against the schema
    xmlDocPtr gpx = gpxDocToXMLDoc(doc);

    if (!validateXMLTree(gpx, gpxSchemaFile))
    {
        xmlFreeDoc(gpx);
        return false;
    }
    xmlFreeDoc(gpx);
    return true;
}

bool writeGPXdoc(GPXdoc *doc, char *fileName)
{ // Writes the GPX doc into a .gpx file
    if (doc == NULL || fileName == NULL)
    {
        return false;
    }
    xmlDocPtr docPtr = gpxDocToXMLDoc(doc);
    xmlSaveFormatFileEnc(fileName, docPtr, "UTF-8", 1);
    xmlFreeDoc(docPtr);
    return true;
}

GPXdoc *createValidGPXdoc(char *fileName, char *gpxSchemaFile)
{ // validates gpx doc against an xsd schema, then writes returns the valid gpx document

    if (fileName == NULL || gpxSchemaFile == NULL)
    {
        return NULL;
    }

    if (validateLibXmlTree(fileName, gpxSchemaFile) == true)
    {
        GPXdoc *gpxObj = createGPXdoc(fileName);
        return gpxObj;
    }

    return NULL;
}

char *GPXdocToString(GPXdoc *doc)
{ // creates a readable format for a gpx object
    if (doc == NULL)
    {
        return NULL;
    }
    char *waypoints = toString(doc->waypoints);
    char *routes = toString(doc->routes);
    char *tracks = toString(doc->tracks);
    int len = strlen(doc->namespace) + strlen(doc->creator) + strlen(waypoints) + strlen(routes) + strlen(tracks) + 100;
    char *tmpStr = malloc(len);

    sprintf(tmpStr, "\nNamespace: %s Creator: %s Version: %0.1f \n \nWaypoints:%s \nRoutes: %s \nTracks:\n %s", doc->namespace, doc->creator, doc->version, waypoints, routes, tracks);
    free(waypoints);
    free(routes);
    free(tracks);
    return tmpStr;
}

void deleteGPXdoc(GPXdoc *doc)
{ // frees all associated memory with the GPXDoc object
    GPXdoc *tmpDoc;
    if (doc == NULL)
    {
        return;
    }
    tmpDoc = doc;
    freeList(tmpDoc->tracks);
    freeList(tmpDoc->routes);
    freeList(tmpDoc->waypoints);
    free(tmpDoc->creator);
    free(tmpDoc);
}

int countWptGPX(ListIterator *li)
{ // sums all the gpx data contained within a waypoint
    Waypoint *tmpWpt;
    int count = 0;
    if (li == NULL)
    {
        return 0;
    }
    while ((tmpWpt = nextElement(li)) != NULL)
    {
        if (strcmp(tmpWpt->name, ""))
        {
            count++;
        }
        count += getLength(tmpWpt->otherData);
    }
    return count;
}

int countRteGPX(ListIterator *li)
{ // sums all the gpx data contained in a route and its children
    int count = 0;
    Route *tmpRte;
    if (li == NULL)
    {
        return 0;
    }
    while ((tmpRte = nextElement(li)) != NULL)
    {
        if (strcmp(tmpRte->name, ""))
        {
            count++;
        }
        // for tags with the name "rtept"
        ListIterator wptIter = createIterator(tmpRte->waypoints);
        count += countWptGPX(&wptIter);
        count += getLength(tmpRte->otherData);
    }
    return count;
}

int countTrkGPX(ListIterator *li)
{ // sums all the gpx data within a track and its children
    int count = 0;
    if (li == NULL)
    {
        return 0;
    }
    Track *tmpTrk;
    TrackSegment *tmpSeg;

    while ((tmpTrk = nextElement(li)) != NULL)
    {
        if (strcmp(tmpTrk->name, ""))
        {
            count++;
        }
        ListIterator segIter = createIterator(tmpTrk->segments);
        while ((tmpSeg = nextElement(&segIter)) != NULL)
        {
            ListIterator wptIter = createIterator(tmpSeg->waypoints);
            count += countWptGPX(&wptIter);
        }
        count += getLength(tmpTrk->otherData);
    }

    return count;
}

int getNumWaypoints(const GPXdoc *doc)
{ // gets number of waypoints in the gpx object
    if (doc == NULL)
    {
        return 0;
    }
    return getLength(doc->waypoints);
}

int getNumRoutes(const GPXdoc *doc)
{ // gets the number of routes in the gpx object
    if (doc == NULL)
    {
        return 0;
    }
    return getLength(doc->routes);
}

int getNumTracks(const GPXdoc *doc)
{ // gets the number of tracks within the gpx object
    if (doc == NULL)
    {
        return 0;
    }
    return getLength(doc->tracks);
}

int getNumSegments(const GPXdoc *doc)
{ // gets number of track segments within the gpx object
    int count = 0;
    if (doc == NULL)
    {
        return 0;
    }
    ListIterator trkIter = createIterator(doc->tracks);
    Track *tmpTrk;

    while ((tmpTrk = nextElement(&trkIter)) != NULL)
    {
        count += getLength(tmpTrk->segments);
    }
    return count;
}

int getNumGPXData(const GPXdoc *doc)
{ // gets the number of gpx data objects stored in the gpx object
    // gets number of non-empty name tags in the document
    if (doc == NULL)
    {
        return 0;
    }
    int count = 0;

    ListIterator wptIter = createIterator(doc->waypoints);
    ListIterator rteIter = createIterator(doc->routes);
    ListIterator trkIter = createIterator(doc->tracks);

    count += countWptGPX(&wptIter);
    count += countRteGPX(&rteIter);
    count += countTrkGPX(&trkIter);
    return count;
}

Waypoint *getWaypoint(const GPXdoc *doc, char *name)
{ // finds a specific waypoint, given a name
    // If two of the same name exist, returns the first wpt with the name
    if (doc == NULL || name == NULL)
    {
        return NULL;
    }
    Waypoint *tmpWpt;
    ListIterator wptIter = createIterator(doc->waypoints);
    while ((tmpWpt = nextElement(&wptIter)) != NULL)
    {
        if (!strcmp(tmpWpt->name, name))
        {
            return tmpWpt;
        }
    }
    return NULL;
}

Track *getTrack(const GPXdoc *doc, char *name)
{ /*Finds a specific Trk object, given its name
* In the event two of the same name exist, returns the first one
*/
    if (doc == NULL || name == NULL)
    {
        return NULL;
    }
    Track *tmpTrk;
    ListIterator trkIter = createIterator(doc->tracks);
    while ((tmpTrk = nextElement(&trkIter)) != NULL)
    {
        if (!strcmp(tmpTrk->name, name))
        {
            return tmpTrk;
        }
    }
    return NULL;
}

Route *getRoute(const GPXdoc *doc, char *name)
{ /* Finds a specific route, given its name
* In the event that two routes of the same name exist, returns the first found
*/
    if (doc == NULL || name == NULL)
    {
        return NULL;
    }
    Route *tmpRte;
    ListIterator rteIter = createIterator(doc->routes);
    while ((tmpRte = nextElement(&rteIter)) != NULL)
    {
        if (!strcmp(tmpRte->name, name))
        {
            return tmpRte;
        }
    }
    return NULL;
}

float haversineDistance(double lat1, double lat2, double long1, double long2)
{ // calculates total distance between two sets of lat + long coordtinates using haversine's formula

    if (!verifyLat(lat1, lat2) || !verifyLong(long1, long2))
    {
        return 0.0;
    }
    float r = 6371 * 1000; // 6371km * 1000m
    float deltaLat = (lat2 - lat1) * (M_PI / 180);
    float deltaLong = (long2 - long1) * (M_PI / 180);
    float angle1 = lat1 * (M_PI / 180), angle2 = lat2 * (M_PI / 180);

    float a = pow(sin(deltaLat / 2), 2) + cos(angle1) * cos(angle2) * pow(sin(deltaLong / 2), 2);
    float c = 2 * atan2(sqrt(a), sqrt(1 - a));

    return r * c;
}

// A2 Required Functions

float getRouteLen(const Route *rt)
{ // this function gets the length of a route by calculating the distance between waypoints using haversine formula
    if (rt == NULL)
    {
        return 0.0;
    }
    float totalLen = 0.0;
    ListIterator rteIter = createIterator(rt->waypoints);
    Waypoint *prevWpt, *nextWpt;
    prevWpt = nextElement(&rteIter);
    while ((nextWpt = nextElement(&rteIter)) != NULL)
    {
        totalLen += haversineDistance(prevWpt->latitude, nextWpt->latitude, prevWpt->longitude, nextWpt->longitude);
        prevWpt = nextWpt;
    }
    return totalLen;
}

float getSegmentLen(const TrackSegment *ts)
{ // this function gets the total length of a specific track segment
    float segmentLen = 0.0;
    ListIterator li = createIterator(ts->waypoints);
    Waypoint *tmpWpt, *prevWpt;
    prevWpt = nextElement(&li);
    while ((tmpWpt = nextElement(&li)) != NULL)
    {
        segmentLen += haversineDistance(prevWpt->latitude, tmpWpt->latitude, prevWpt->longitude, tmpWpt->longitude);
        prevWpt = tmpWpt;
    }
    return segmentLen;
}

float getTrackLen(const Track *tr)
{ // this function gets the length of a track, by the sum of all the segments (see above function)
    float trackLen = 0.0;
    if (tr == NULL)
    {
        return 0.0;
    }
    ListIterator li = createIterator(tr->segments);
    TrackSegment *currSeg, *prevSeg = NULL;
    if (getLength(tr->segments) == 1)
    { // sum of only the segment
        currSeg = getFromFront(tr->segments);
        trackLen += getSegmentLen(currSeg);
        return trackLen;
    }
    else
    { // use the modified formula, seg1Len + seg2Len + dist between seg1 tail and seg2 head
        Waypoint *seg1Tail, *seg2Head;
        while ((currSeg = nextElement(&li)) != NULL)
        {
            if (prevSeg != NULL)
            {
                seg1Tail = getFromBack(prevSeg->waypoints);
                seg2Head = getFromFront(currSeg->waypoints);
                trackLen += haversineDistance(seg1Tail->latitude, seg2Head->latitude, seg1Tail->longitude, seg2Head->longitude);
            }
            trackLen += getSegmentLen(currSeg);
            prevSeg = currSeg;
        }
    }
    return trackLen;
}

float round10(float len)
{ // gets number of zeroes, divides by number, rounds to nearest 10 and then multiplies
    return round(len / 10) * 10;
}

int numRoutesWithLength(const GPXdoc *doc, float len, float delta)
{
    int numRoutes = 0;
    if (doc == NULL || len < 0.0 || delta < 0.0)
    {
        return 0;
    }
    ListIterator rteIter = createIterator(doc->routes);
    Route *rte;
    while ((rte = nextElement(&rteIter)) != NULL)
    {
        if (fabs(getRouteLen(rte) - len) <= delta)
        {
            ++numRoutes;
        }
    }
    return numRoutes;
}

int numTracksWithLength(const GPXdoc *doc, float len, float delta)
{
    int numTracks = 0;
    if (doc == NULL || len < 0.0 || delta < 0.0)
    {
        return 0;
    }
    ListIterator trkIter = createIterator(doc->tracks);
    Track *trk;
    while ((trk = nextElement(&trkIter)) != NULL)
    {
        if (fabs(getTrackLen(trk) - len) <= delta)
        {
            ++numTracks;
        }
    }
    return numTracks;
}

bool isLoopRoute(const Route *route, float delta)
{ // grabs the head and tail, runs haversine and checks if the distance between waypoints is within the delta provided
    if (route == NULL || delta < 0.0)
    { // cant be a loop if there isnt a valid route or delta  :thinking:
        return false;
    }
    if (getLength(route->waypoints) < 4)
    { // cant be a loop unless its got 4 points
        return false;
    }
    Waypoint *head = getFromFront(route->waypoints), *tail = getFromBack(route->waypoints);
    float distBetween = haversineDistance(head->latitude, tail->latitude, head->longitude, tail->longitude);
    if (distBetween - delta > 0)
    { // if the distance is not within the delta, its not a loop
        return false;
    }
    return true;
}

bool isLoopTrack(const Track *tr, float delta)
{ // determines if a track is considered a loop, by taking the distance between the head of the first segment and tail of the last segment
    if (tr == NULL || delta < 0.0)
    {
        return false;
    }
    ListIterator li = createIterator(tr->segments);
    TrackSegment *headSeg = getFromFront(tr->segments), *tailSeg = getFromBack(tr->segments), *tmpSeg;
    Waypoint *head = getFromFront(headSeg->waypoints), *tail = getFromBack(tailSeg->waypoints);
    float dist = haversineDistance(head->latitude, tail->latitude, head->longitude, tail->longitude);
    int numWpt = 0;
    while ((tmpSeg = nextElement(&li)) != NULL)
    {
        numWpt += getLength(tmpSeg->waypoints);
    }
    if (numWpt < 4)
    {
        return false;
    }
    if (dist <= delta)
    {
        return true;
    }
    return false;
}

void deleteList(void *data)
{
    return;
}

float lengthBetween(Waypoint *wpt, float lat, float lon)
{
    return haversineDistance(wpt->latitude, lat, wpt->longitude, lon);
}

List *getRoutesBetween(const GPXdoc *doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta)
{ // gets all routes in between a set of two points
    if (doc == NULL || !verifyLong(sourceLong, destLong) || !verifyLat(sourceLat, destLat) || delta < 0.0)
    {
        return NULL;
    }
    List *routesBetween = initializeList(&routeToString, &deleteList, &compareRoutes);
    ListIterator rteIter = createIterator(doc->routes);
    Waypoint *headPt, *tailPt;
    Route *rte;
    //float targetDist = haversineDistance(sourceLat, destLat, sourceLong, destLong), rteDist;
    while ((rte = nextElement(&rteIter)) != NULL)
    { // search all routes
        headPt = getFromFront(rte->waypoints);
        tailPt = getFromBack(rte->waypoints);
        //rteDist = haversineDistance(headPt->latitude, tailPt->latitude, headPt->longitude, tailPt->longitude);
        //printf("delta: %f Startlen %f\n", delta, lengthBetween(headPt, sourceLat, sourceLong));
        //printf("delta: %f Endlen %f\n", delta, lengthBetween(tailPt, destLat, destLong));
        if (lengthBetween(headPt, sourceLat, sourceLong) <= delta && lengthBetween(tailPt, destLat, destLong) <= delta)
        {
            //printf("Inserted Route \n");
            insertFront(routesBetween, rte);
        }
    }
    if (getLength(routesBetween) == 0)
    {
        freeList(routesBetween);
        routesBetween = NULL;
    }
    return routesBetween;
}

List *getTracksBetween(const GPXdoc *doc, float sourceLat, float sourceLong, float destLat, float destLong, float delta)
{ // gets a list of tracks between a set of two points
    if (doc == NULL || !verifyLong(sourceLong, destLong) || !verifyLat(sourceLat, destLat) || delta < 0.0)
    {
        return NULL;
    }
    List *tracksBetween = initializeList(&trackToString, &deleteList, &compareTracks);
    ListIterator trkIter = createIterator(doc->tracks);
    TrackSegment *headSeg, *tailSeg;
    Track *trk;
    Waypoint *head, *tail;
    // float targetDist = haversineDistance(sourceLat, destLat, sourceLong, destLong), calculatedDistance = 0.0;
    while ((trk = nextElement(&trkIter)) != NULL)
    {
        headSeg = getFromFront(trk->segments);
        tailSeg = getFromBack(trk->segments);
        head = getFromFront(headSeg->waypoints);
        tail = getFromBack(tailSeg->waypoints);
        //calculatedDistance = haversineDistance(head->latitude, tail->latitude, head->longitude, tail->longitude);
        if (lengthBetween(head, sourceLat, sourceLong) <= delta && lengthBetween(tail, destLat, destLong) <= delta)
        {
            insertBack(tracksBetween, trk);
        }
    }
    if (getLength(tracksBetween) == 0)
    {
        freeList(tracksBetween);
        return NULL;
    }
    return tracksBetween;
}

char *gpxDatatoJSON(const GPXData *gData)
{
    char *JSONStr = malloc(sizeof(char) * 4);
    if (gData == NULL)
    {
        sprintf(JSONStr, "%s", "{}");
        return JSONStr;
    }
    int i = 0;
    JSONStr = realloc(JSONStr, sizeof(char) * strlen(gData->name) + strlen(gData->value) + 100);
    sprintf(JSONStr, "{\"name\":\"%s\",\"value\":\"%s\"}", gData->name, gData->value);
    while (i < strlen(JSONStr) + 1)
    {
        if (JSONStr[i] == '\n')
        {
            JSONStr[i] = ' ';
        }
        i++;
    }
    return JSONStr;
}

char *gpxDataListtoJSON(const List *list)
{
    char *JSONStr = malloc(sizeof(char) * 4);
    if (list == NULL)
    {
        sprintf(JSONStr, "%s", "[]");
        return JSONStr;
    }
    char *tmpStr = calloc(10000, sizeof(char)), *str;
    List *lt = (List *)list;
    ListIterator li = createIterator(lt);
    GPXData *gd;
    while ((gd = nextElement(&li)) != NULL)
    {
        str = gpxDatatoJSON((const GPXData *)gd);
        tmpStr = realloc(tmpStr, strlen(tmpStr) + strlen(str) + 200);
        strcat(tmpStr, str);
        strcat(tmpStr, ",");
        free(str);
    }
    tmpStr[strlen(tmpStr) - 1] = '\0';
    JSONStr = realloc(JSONStr, sizeof(char) * strlen(JSONStr) + strlen(tmpStr));
    sprintf(JSONStr, "[%s]", tmpStr);
    free(tmpStr);
    return JSONStr;
}

char *waypointToJSON(const Waypoint *wpt)
{
    char *JSONStr = malloc(sizeof(char) * 4);
    if (wpt == NULL)
    {
        sprintf(JSONStr, "%s", "{}");
        return JSONStr;
    }
    char *otherData = gpxDataListtoJSON(wpt->otherData);
    JSONStr = realloc(JSONStr, strlen(JSONStr) + strlen(otherData) + 1000);
    sprintf(JSONStr, "{\"name\":\"%s\",\"lat\":%f,\"lon\":%f,\"otherData\":%s}", wpt->name, wpt->latitude, wpt->longitude, otherData);
    free(otherData);
    return JSONStr;
}

char *waypointListToJSON(const List *list)
{
    char *JSONStr = malloc(sizeof(char) * 10000);
    if (list == NULL)
    {
        sprintf(JSONStr, "%s", "[]");
        return JSONStr;
    }
    char *tmpStr = calloc(10000, sizeof(char)), *str;
    List *lt = (List *)list;
    ListIterator li = createIterator(lt);
    Waypoint *wpt;
    while ((wpt = nextElement(&li)) != NULL)
    {
        str = waypointToJSON((const Waypoint *)wpt);
        tmpStr = realloc(tmpStr, strlen(tmpStr) + strlen(str) + 100);
        strcat(tmpStr, str);
        strcat(tmpStr, ",");
        free(str);
    }
    tmpStr[strlen(tmpStr) - 1] = ' ';
    //printf("%s \n", tmpStr);
    JSONStr = realloc(JSONStr, sizeof(char) * strlen(JSONStr) + strlen(tmpStr));
    sprintf(JSONStr, "[%s]", tmpStr);
    free(tmpStr);
    return JSONStr;
}

char *trackToJSON(const Track *tr)
{ // takes a track and converts it to a json format
    char *JSONstr = malloc(sizeof(char) * 10000);
    if (tr == NULL)
    {
        sprintf(JSONstr, "%s", "{}");
        return JSONstr;
    }
    char loop[7] = "false";
    if (isLoopTrack(tr, 10.0))
    {
        strcpy(loop, "true");
    }
    char *otherData = gpxDataListtoJSON(tr->otherData);
    int numWpt = 0;
    ListIterator li = createIterator(tr->segments);
    TrackSegment *ts;
    while ((ts = nextElement(&li)) != NULL)
    {
        numWpt += getLength(ts->waypoints);
    }
    JSONstr = realloc(JSONstr, sizeof(char) * strlen(loop) + strlen(otherData) + 150);
    sprintf(JSONstr, "{\"name\":\"%s\",\"len\":%.1f,\"numPoints\":%d,\"loop\":%s,\"otherData\":%s}", tr->name, round10(getTrackLen(tr)), numWpt, loop, otherData);
    free(otherData);
    return JSONstr;
}

char *routeToJSON(const Route *rt)
{ // takes a route and converts it to json format
    char *JSONstr = malloc(sizeof(char) * 4);
    if (rt == NULL)
    {
        sprintf(JSONstr, "%s", "{}");
        return JSONstr;
    }
    char loop[7] = "false";
    if (isLoopRoute(rt, 10.0))
    {
        strcpy(loop, "true");
    }
    char *otherData = gpxDataListtoJSON(rt->otherData);
    char *waypoints = waypointListToJSON(rt->waypoints);
    //printf("Waypoints: %s \n", waypoints);
    JSONstr = realloc(JSONstr, sizeof(char) * strlen(loop) + strlen(otherData) + strlen(waypoints) + 150);
    sprintf(JSONstr, "{\"name\":\"%s\",\"numPoints\":%d,\"len\":%.1f,\"loop\":%s,\"waypoints\":%s,\"otherData\":%s}", rt->name, getLength(rt->waypoints), round10(getRouteLen(rt)), loop, waypoints, otherData);
    //printf("%s \n", JSONstr);
    free(otherData);
    free(waypoints);
    return JSONstr;
}

char *routeListToJSON(const List *list)
{ // takes a list of routes and converts them to json format
    char *JSONstr = malloc(sizeof(char) * 10000);
    if (list == NULL)
    {
        sprintf(JSONstr, "%s", "[]");
        return JSONstr;
    }
    char *tmpStr = calloc(10000, sizeof(char)), *str;
    List *rteList = (List *)list;
    ListIterator rteIter = createIterator(rteList);
    Route *rte;
    while ((rte = nextElement(&rteIter)) != NULL)
    {
        str = routeToJSON((const Route *)rte);
        strcat(tmpStr, str);
        strcat(tmpStr, ",");
        free(str);
    }
    if (tmpStr[strlen(tmpStr - 1)] == ',')
    {
        tmpStr[strlen(tmpStr - 1)] = '\0';
    }
    tmpStr[strlen(tmpStr) - 1] = '\0';
    sprintf(JSONstr, "[%s]", tmpStr);
    free(tmpStr);
    return JSONstr;
}

char *trackListToJSON(const List *list)
{ // takes a list of tracks and then converts them to JSON format
    char *JSONstr = malloc(sizeof(char) * 10000);
    if (list == NULL)
    {
        sprintf(JSONstr, "%s", "[]");
        return JSONstr;
    }
    char *tmpStr = calloc(10000, sizeof(char)), *str;
    List *trkList = (List *)list;
    ListIterator trkIter = createIterator(trkList);
    Track *trk;
    while ((trk = nextElement(&trkIter)) != NULL)
    {
        str = trackToJSON((const Track *)trk);
        strcat(tmpStr, str);
        strcat(tmpStr, ",");
        free(str);
    }
    tmpStr[strlen(tmpStr) - 1] = '\0';
    sprintf(JSONstr, "[%s]", tmpStr);
    free(tmpStr);
    return JSONstr;
}

char *GPXtoJSON(const GPXdoc *gpx)
{ // takes a gpxdoc and converts it to a json format
    char *JSONstr = malloc(sizeof(char) * 10000);
    if (gpx == NULL)
    {
        sprintf(JSONstr, "%s", "{}");
        return JSONstr;
    }
    sprintf(JSONstr, "{\"version\":%0.1f,\"creator\":\"%s\",\"numWaypoints\":%d,\"numRoutes\":%d,\"numTracks\":%d}",
            gpx->version, gpx->creator, getNumWaypoints(gpx), getNumRoutes(gpx), getNumTracks(gpx));
    return JSONstr;
}

char *GPXDoctoJSON(const GPXdoc *gpx)
{ // takes a gpxdoc and converts it to a json format
    char *JSONstr = malloc(sizeof(char) * 20000);
    if (gpx == NULL)
    {
        sprintf(JSONstr, "%s", "{}");
        return JSONstr;
    }
    char *routes = routeListToJSON(gpx->routes);
    char *tracks = trackListToJSON(gpx->tracks);
    //char *waypoints = waypointListToJSON(gpx->waypoints);
    JSONstr = realloc(JSONstr, strlen(JSONstr) + strlen(routes) + strlen(tracks) + 150);
    sprintf(JSONstr, "{\"version\":%0.1f,\"creator\":\"%s\",\"numWaypoints\":%d,\"numRoutes\":%d,\"numTracks\":%d,\"routes\":%s,\"tracks\":%s}",
            gpx->version, gpx->creator, getNumWaypoints(gpx), getNumRoutes(gpx), getNumTracks(gpx), routes, tracks);
    free(routes);
    free(tracks);
    //free(waypoints);
    return JSONstr;
}

/** Bonus A2 **/

void addWaypoint(Route *rt, Waypoint *pt)
{ // adds a waypoint into the list of waypoints in a route
    if (rt == NULL || pt == NULL)
    {
        return;
    }
    insertBack(rt->waypoints, pt);
}

void addRoute(GPXdoc *doc, Route *rt)
{ // adds a route into the list of routes in the gpx doc
    if (doc == NULL || rt == NULL)
    {
        return;
    }
    insertBack(doc->routes, rt);
}

GPXdoc *JSONtoGPX(const char *gpxString)
{ // takes a json and converts it back to gpx doc format
    if (gpxString == NULL)
    {
        return NULL;
    }
    GPXdoc *convertedDoc = calloc(1, sizeof(GPXdoc));
    convertedDoc->tracks = initializeList(&trackToString, &deleteTrack, &compareTracks);
    convertedDoc->routes = initializeList(&routeToString, &deleteRoute, &compareRoutes);
    convertedDoc->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    convertedDoc->creator = calloc(1, sizeof(char));
    strcpy(convertedDoc->namespace, "http://www.topografix.com/GPX/1/1");
    char *gpxJSON = malloc(sizeof(char) * strlen(gpxString) + 1), *token;
    strcpy(gpxJSON, gpxString);
    token = strtok(gpxJSON, "{},:\"");
    int i = 0;
    while (token != NULL)
    {
        if (i == 1)
        {
            convertedDoc->version = atof(token);
        }
        if (i == 3)
        {
            convertedDoc->creator = realloc(convertedDoc->creator, strlen(token) + 1);
            strcpy(convertedDoc->creator, token);
        }
        token = strtok(NULL, "{},:\"");
        ++i;
    }
    return convertedDoc;
}

Waypoint *JSONtoWaypoint(const char *gpxString)
{ // takes a json and converts it to waypoint format
    if (gpxString == NULL)
    {
        return NULL;
    }
    Waypoint *wpt = calloc(1, sizeof(Waypoint));
    wpt->name = calloc(1, sizeof(char));
    wpt->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
    char *wptJSON = malloc(sizeof(char) * strlen(gpxString) + 1), *token;
    strcpy(wptJSON, gpxString);
    token = strtok(wptJSON, "{},:\"");
    int i = 0;
    while (token != NULL)
    {
        if (i == 1)
        {
            wpt->latitude = atof(token);
        }
        if (i == 3)
        {
            wpt->longitude = atof(token);
        }
        token = strtok(NULL, "{},:\"");
        ++i;
    }
    return wpt;
}

Route *JSONtoRoute(const char *gpxString)
{ // takes a route and converts it to a route format
    if (gpxString == NULL)
    {
        return NULL;
    }
    Route *rte = calloc(1, sizeof(Route));
    rte->name = calloc(1, sizeof(char));
    rte->waypoints = initializeList(&waypointToString, &deleteWaypoint, &compareWaypoints);
    rte->otherData = initializeList(&gpxDataToString, &deleteGpxData, &compareGpxData);
    char *rteJSON = malloc(sizeof(char) * strlen(gpxString) + 1), *token;
    strcpy(rteJSON, gpxString);
    token = strtok(rteJSON, "{}:,\"");
    int i = 0;
    while (token != NULL)
    {
        if (i == 1)
        {
            rte->name = realloc(rte->name, strlen(token) + 1);
            strcpy(rte->name, token);
        }
        token = strtok(NULL, "{}:,\"");
        ++i;
    }
    return rte;
}

/** Intermediary functions for A3 **/
/* general parsing for all gpx files... */
char *GPXFiletoJSON(char *filename, char *gpxschemafile)
{

    GPXdoc *doc = createValidGPXdoc(filename, gpxschemafile);
    if (filename == NULL || gpxschemafile == NULL || doc == NULL)
    {
        return NULL;
    }
    char *JSONStr = malloc(sizeof(char) * 10000);
    char *docJSON = GPXtoJSON(doc);
    JSONStr = realloc(JSONStr, strlen(docJSON));
    sprintf(JSONStr, "%s", docJSON);
    free(docJSON);
    deleteGPXdoc(doc);
    return JSONStr;
}

/* parses a specific gpx file and returns more detailed info */
char *GPXFiletoJSONDetailed(char *filename, char *gpxschemafile)
{
    GPXdoc *doc = createValidGPXdoc(filename, gpxschemafile);
    if (doc == NULL || filename == NULL || gpxschemafile == NULL)
    {
        return NULL;
    }
    char *JSONStr = malloc(sizeof(char) * 10000);
    char *docJSON = GPXDoctoJSON(doc);
    JSONStr = realloc(JSONStr, strlen(docJSON) + 10);
    sprintf(JSONStr, "%s", docJSON);
    deleteGPXdoc(doc);
    free(docJSON);
    return JSONStr;
}

/* Validates and then Writes Gpx Doc, to a file, from a JSON*/
bool createDotGPX(char *data, char *filename, char *gpxschemafile)
{
    if (data == NULL || filename == NULL || gpxschemafile == NULL)
    {
        return false;
    }
    GPXdoc *doc = JSONtoGPX(data);
    if (doc == NULL)
    { // doc is null
        deleteGPXdoc(doc);
        return false;
    }
    if (validateGPXDoc(doc, gpxschemafile))
    { // doc isnt validated
        return writeGPXdoc(doc, filename);
    }
    return false;
}
/* Finds route, renames it, writes to the gpx file */
bool updateRouteName(char *newName, char *componentName, char *filename, char *gpxschemafile)
{
    if (newName == NULL || componentName == NULL || filename == NULL || gpxschemafile == NULL)
    {
        return false;
    }
    GPXdoc *doc = createValidGPXdoc(filename, gpxschemafile);
    if (doc == NULL)
    {
        return false;
    }
    Route *rte = getRoute(doc, componentName);
    if (rte == NULL)
    { // no route found
        deleteGPXdoc(doc);
        return false;
    }
    rte->name = realloc(rte->name, sizeof(char) * strlen(newName) + 20);
    strcpy(rte->name, newName);
    bool ret = writeGPXdoc(doc, filename);
    deleteGPXdoc(doc);
    return ret;
}
/* Finds track, renames it and then writes to the corresponding gpx */
bool updateTrackName(char *newName, char *componentName, char *filename, char *gpxschemafile)
{
    if (newName == NULL || componentName == NULL || filename == NULL || gpxschemafile == NULL)
    {
        return false;
    }
    GPXdoc *doc = createValidGPXdoc(filename, gpxschemafile);
    if (doc == NULL)
    {
        return false;
    }
    Track *tr = getTrack(doc, componentName);
    if (tr == NULL)
    {
        return false;
    }
    tr->name = realloc(tr->name, sizeof(char) * strlen(newName) + 20);
    strcpy(tr->name, newName);
    bool ret = writeGPXdoc(doc, filename);
    deleteGPXdoc(doc);
    return ret;
}

char *getRteGpxData(char *rteName, char *filename, char *gpxschemafile)
{
    if (rteName == NULL || filename == NULL || gpxschemafile == NULL)
    {
        char *JSONStr = malloc(sizeof(char) * 4);
        sprintf(JSONStr, "%s", "[]");
        return JSONStr;
    }
    GPXdoc *doc = createValidGPXdoc(filename, gpxschemafile);
    if (doc == NULL)
    {
        char *JSONStr = malloc(sizeof(char) * 4);
        sprintf(JSONStr, "%s", "[]");
        return JSONStr;
    }
    Route *rte = getRoute(doc, rteName);
    if (rte == NULL)
    {
        char *JSONStr = malloc(sizeof(char) * 4);
        sprintf(JSONStr, "%s", "[]");
        return JSONStr;
    }
    char *JSONStr = gpxDataListtoJSON(rte->otherData);
    return JSONStr;
}

char *getTrkGpxData(char *trkName, char *filename, char *gpxschemafile)
{
    if (trkName == NULL || filename == NULL || gpxschemafile == NULL)
    {
        char *JSONStr = malloc(sizeof(char) * 4);
        sprintf(JSONStr, "%s", "[]");
        return JSONStr;
    }
    GPXdoc *doc = createValidGPXdoc(filename, gpxschemafile);
    if (doc == NULL)
    {
        char *JSONStr = malloc(sizeof(char) * 4);
        sprintf(JSONStr, "%s", "[]");
        return JSONStr;
    }
    Track *trk = getTrack(doc, trkName);
    if (trk == NULL)
    {
        char *JSONStr = malloc(sizeof(char) * 4);
        sprintf(JSONStr, "%s", "[]");
        return JSONStr;
    }
    char *JSONStr = gpxDataListtoJSON(trk->otherData);
    return JSONStr;
}

bool addNewRoute(char *waypoints, char *route, char *filename, char *gpxschemafile)
{
    if (waypoints == NULL || route == NULL || filename == NULL || gpxschemafile == NULL)
    {
        return false;
    }
    GPXdoc *doc = createValidGPXdoc(filename, gpxschemafile);
    if (doc == NULL)
    {
        return false;
    }
    Route *rte = JSONtoRoute(route);
    Waypoint *tmp;
    //printf("%s \n", waypoints);
    char *token;
    token = strtok(waypoints, "-"); // breaks waypoint into its corresponding JSON
    char *tmpStr = malloc(sizeof(char) * 100);
    char **strings = malloc(sizeof(char *) * 10);
    int numTokens = 0;
    while (token != NULL)
    {
        strings = realloc(strings, sizeof(char *) * numTokens + 1);
        strings[numTokens] = malloc(sizeof(char) * strlen(token));
        strcpy(strings[numTokens], token);
        token = strtok(NULL, "-");
        numTokens++;
    }
    for (int i = 0; i < numTokens; i++)
    {
        //printf("%s \n", strings[i]);
        tmp = JSONtoWaypoint(strings[i]);
        insertBack(rte->waypoints, tmp);
        free(strings[i]);
    }
    insertBack(doc->routes, rte);
    free(tmpStr);
    free(strings);
    bool writeStatus = writeGPXdoc(doc, filename);
    return writeStatus;
}

char *routesBetweenToJSON(char *filename, char *gpxschemafile, float sourceLat, float sourceLong, float destLat, float destLong, float delta)
{
    if (filename == NULL || gpxschemafile == NULL)
    {
        char *JSONStr = malloc(sizeof(char) * 4);
        sprintf(JSONStr, "%s", "[]");
        return JSONStr;
    }
    GPXdoc *doc = createValidGPXdoc(filename, gpxschemafile);
    if (doc == NULL)
    {
        char *JSONStr = malloc(sizeof(char) * 4);
        sprintf(JSONStr, "%s", "[]");
        return JSONStr;
    }
    if (doc->routes == NULL)
    {
        char *JSONStr = malloc(sizeof(char) * 4);
        sprintf(JSONStr, "%s", "[]");
        return JSONStr;
    }
    List *rtBetween = getRoutesBetween(doc, sourceLat, sourceLong, destLat, destLong, delta);
    if (rtBetween == NULL)
    {
        char *JSONStr = malloc(sizeof(char) * 4);
        sprintf(JSONStr, "%s", "[]");
        return JSONStr;
    }
    char *JSONStr = routeListToJSON(rtBetween);
    return JSONStr;
}

char *tracksBetweenToJSON(char *filename, char *gpxschemafile, float sourceLat, float sourceLong, float destLat, float destLong, float delta)
{
    if (filename == NULL || gpxschemafile == NULL)
    {
        char *JSONStr = malloc(sizeof(char) * 4);
        sprintf(JSONStr, "%s", "[]");
        return JSONStr;
    }
    GPXdoc *doc = createValidGPXdoc(filename, gpxschemafile);
    if (doc == NULL)
    {
        char *JSONStr = malloc(sizeof(char) * 4);
        sprintf(JSONStr, "%s", "[]");
        return JSONStr;
    }
    if (doc->tracks == NULL)
    {
        char *JSONStr = malloc(sizeof(char) * 4);
        sprintf(JSONStr, "%s", "[]");
        return JSONStr;
    }
    List *trkBetween = getTracksBetween(doc, sourceLat, sourceLong, destLat, destLong, delta);
    if (trkBetween == NULL)
    {
        char *JSONStr = malloc(sizeof(char) * 4);
        sprintf(JSONStr, "%s", "[]");
        return JSONStr;
    }
    char *JSONStr = trackListToJSON(trkBetween);
    return JSONStr;
}

int tracksofLenJSON(char *filename, char *gpxschemafile, float len)
{
    if (filename == NULL || gpxschemafile == NULL || len <= 0.0)
    {
        return 0;
    }
    GPXdoc *doc = createValidGPXdoc(filename, gpxschemafile);
    if (doc == NULL)
    {
        return 0;
    }
    int tracksOfLen = numTracksWithLength(doc, len, 10.0);
    deleteGPXdoc(doc);
    return tracksOfLen;
}

int routesofLenJSON(char *filename, char *gpxschemafile, float len)
{
    if (filename == NULL || gpxschemafile == NULL || len <= 0.0)
    {
        return 0;
    }
    GPXdoc *doc = createValidGPXdoc(filename, gpxschemafile);
    if (doc == NULL)
    {
        return 0;
    }
    int routesOfLen = numRoutesWithLength(doc, len, 10.0);
    deleteGPXdoc(doc);
    return routesOfLen;
}

/** Helpers **/

void deleteGpxData(void *data)
{
    if (data == NULL)
    {
        return;
    }
    GPXData *tmpData = (GPXData *)data;
    free(tmpData);
}

char *gpxDataToString(void *data)
{
    if (data == NULL)
    {
        return NULL;
    }
    GPXData *tmpData = (GPXData *)data;
    char *tmpStr;
    int len = strlen(tmpData->name) + strlen(tmpData->value) + 100;
    tmpStr = malloc(sizeof(char) * len);
    sprintf(tmpStr, "Name: %s Value: %s", tmpData->name, tmpData->value);
    return tmpStr;
}

int compareGpxData(const void *first, const void *second)
{
    if (first == NULL || second == NULL)
    {
        return 0;
    }
    GPXData *firstData = (GPXData *)first;
    GPXData *secondData = (GPXData *)second;
    return strcmp((char *)firstData, (char *)secondData);
}

void deleteWaypoint(void *data)
{
    if (data == NULL)
    {
        return;
    }

    Waypoint *tmpWP;
    tmpWP = (Waypoint *)data;

    freeList(tmpWP->otherData);
    free(tmpWP->name);
    free(tmpWP);
}

char *waypointToString(void *data)
{
    if (data == NULL)
    {
        return NULL;
    }
    char *tmpStr;
    Waypoint *tmpWP = (Waypoint *)data;
    char *gpxData = toString(tmpWP->otherData);
    int len;

    len = strlen(tmpWP->name) + strlen(gpxData) + 100;
    tmpStr = malloc(sizeof(char) * len);
    sprintf(tmpStr, "\n \t \t \t Name: %s Lat: %f Long: %f Waypoint Data: %s ", tmpWP->name, tmpWP->latitude, tmpWP->longitude, gpxData);
    free(gpxData);
    return tmpStr;
}

int compareWaypoints(const void *first, const void *second)
{
    if (first == NULL || second == NULL)
    {
        return 0;
    }
    Waypoint *wp1 = (Waypoint *)first;
    Waypoint *wp2 = (Waypoint *)second;

    return strcmp((char *)wp1->name, (char *)wp2->name);
}

void deleteRoute(void *data)
{
    if (data == NULL)
    {
        return;
    }
    Route *tmpRte = (Route *)data;
    free(tmpRte->name);
    freeList(tmpRte->waypoints);
    freeList(tmpRte->otherData);
    free(tmpRte);
}

char *routeToString(void *data)
{
    if (data == NULL)
    {
        return NULL;
    }
    Route *tmpRte = (Route *)data;
    char *tmpStr;
    char *waypoints = toString(tmpRte->waypoints);
    char *gpxData = toString(tmpRte->otherData);
    int len;

    len = strlen(tmpRte->name) + strlen(gpxData) + strlen(waypoints) + 100;
    tmpStr = malloc(len);

    sprintf(tmpStr, "\n \t Name: %s \n \t Waypoints: %s \n\tRoute Data: %s", tmpRte->name, waypoints, gpxData);
    free(waypoints);
    free(gpxData);
    return tmpStr;
}

int compareRoutes(const void *first, const void *second)
{
    if (first == NULL || second == NULL)
    {
        return 0;
    }
    Route *tmpRte1 = (Route *)first;
    Route *tmpRte2 = (Route *)second;

    return strcmp((char *)tmpRte1->name, (char *)tmpRte2->name);
}

void deleteTrackSegment(void *data)
{
    if (data == NULL)
    {
        return;
    }
    TrackSegment *tmpTSeg = (TrackSegment *)data;
    freeList(tmpTSeg->waypoints);
    free(tmpTSeg);
}

char *trackSegmentToString(void *data)
{
    if (data == NULL)
    {
        return NULL;
    }
    TrackSegment *tmpSeg = (TrackSegment *)data;
    char *tmpStr;
    char *waypoints = toString(tmpSeg->waypoints);
    int len = strlen(waypoints) + 100;
    tmpStr = malloc(sizeof(char) * len);
    sprintf(tmpStr, "\n \n \t \tTrack Segment:\n \t \t \tWaypoints: %s\n", waypoints);
    free(waypoints);
    return tmpStr;
}

int compareTrackSegments(const void *first, const void *second)
{
    if (first == NULL || second == NULL)
    {
        return 0;
    }
    TrackSegment *seg1 = (TrackSegment *)first;
    TrackSegment *seg2 = (TrackSegment *)second;
    return strcmp((char *)seg1, (char *)seg2);
}

void deleteTrack(void *data)
{
    if (data == NULL)
    {
        return;
    }
    Track *tmpTrk = (Track *)data;
    free(tmpTrk->name);
    freeList(tmpTrk->segments);
    freeList(tmpTrk->otherData);
    free(tmpTrk);
}

char *trackToString(void *data)
{
    if (data == NULL)
    {
        return NULL;
    }
    Track *tmpTrk = (Track *)data;

    char *tmpStr;
    char *segments = toString(tmpTrk->segments);
    char *gpxData = toString(tmpTrk->otherData);
    int len = strlen(tmpTrk->name) + strlen(gpxData) + strlen(segments) + 100;

    tmpStr = malloc(sizeof(char) * len);
    sprintf(tmpStr, "\n \t Name: %s\n \t \tSegments: %s \n \t Track Data: %s\n", tmpTrk->name, segments, gpxData);
    free(segments);
    free(gpxData);
    return tmpStr;
}

int compareTracks(const void *first, const void *second)
{
    if (first == NULL || second == NULL)
    {
        return 0;
    }
    Track *firstSegment = (Track *)first;
    Track *secondSegment = (Track *)second;
    return strcmp((char *)firstSegment, (char *)secondSegment);
}
