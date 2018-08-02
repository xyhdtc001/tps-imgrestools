#ifndef _PATH_H_
#define _PATH_H_

#include "basecode/str.h"

class Path
{
public:
   enum Separator
   {
      OsSeparator = '/'
   };

   Path()
      :  mIsDirtyFileName( true ),
         mIsDirtyPath( true )
   {
   }

   Path( const char *file )
      :  mIsDirtyFileName( true ),
         mIsDirtyPath( true )
   {
      _split(file);
   }

   Path( const String &file )
      :  mIsDirtyFileName( true ),
         mIsDirtyPath( true )
   {
      _split(file);
   }

   Path& operator = ( const String &file ) { _split(file); mIsDirtyPath = mIsDirtyFileName = true; return *this; }
   operator String() const { return getFullPath(); }

   bool operator == (const Path& path) const { return getFullPath().equal(path.getFullPath()); }
   bool operator != (const Path& path) const { return !(*this == path); }

   bool isEmpty() const { return getFullPath().isEmpty(); }

   /// Join two path or file name components together.
   static String Join(const String&,String::ValueType,const String&);

   /// Replace all '\' with '/'
   static String CleanSeparators( String path );

   /// Remove "." and ".." relative paths.
   static String CompressPath( String path );

   /// Take two paths and return the relative path between them.
   static Path MakeRelativePath( const Path &makeRelative, const Path &relativeTo, uint32 mode = String::NoCase );
   
   const String& getRoot() const { return mRoot; }
   const String& getPath() const { return mPath; }
   const String& getFileName() const { return mFile; }
   const String& getExtension() const { return mExt; }

   const String& getFullFileName() const;
   const String& getFullPath() const;
   
   /// Returns the full file path without the volume root.
   String getFullPathWithoutRoot() const;

   /// Returns the root and path.
   String getRootAndPath() const;

   const String& setRoot(const String &s);
   const String& setPath(const String &s);
   const String& setFileName(const String &s);
   const String& setExtension(const String &s);
   
   uint32 getDirectoryCount() const;
   String getDirectory(uint32) const;
   
   bool isDirectory() const;
   bool isRelative() const;
   bool isAbsolute() const;
   
   /// Appends the argument's path component to the object's
   /// path component. The object's root, filename and
   /// extension are unaffected.
   bool appendPath(const Path &path);

private:
   String   mRoot;
   String   mPath;
   String   mFile;
   String   mExt;
   
   mutable String   mFullFileName;
   mutable String   mFullPath;

   mutable bool  mIsDirtyFileName;
   mutable bool  mIsDirtyPath;
   
   void _split(String name);
   String _join() const;
};

String PathToPlatform(String file);
String PathToOS(String file);

#endif

