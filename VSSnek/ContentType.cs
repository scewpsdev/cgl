using Microsoft.VisualStudio.LanguageServer.Client;
using Microsoft.VisualStudio.Utilities;
using System.ComponentModel.Composition;

internal static class MyContentType
{
	[Export]
	[Name("sneklang")]
	[BaseDefinition(CodeRemoteContentDefinition.CodeRemoteContentTypeName)]
	internal static ContentTypeDefinition MyDslContentType;

	[Export]
	[FileExtension(".src")]
	[ContentType("sneklang")]
	internal static FileExtensionToContentTypeDefinition MyDslFileExtension;
}

