#include "Combat/TalentTree/AtlasTalentTreeData.h"

const FAtlasTalentNodeDefinition* UAtlasTalentTreeData::FindTalentDefinition(const FGameplayTag TalentTag) const
{
	if (!TalentTag.IsValid())
	{
		return nullptr;
	}

	return TalentNodes.FindByPredicate(
		[TalentTag](const FAtlasTalentNodeDefinition& TalentNode)
		{
			return TalentNode.TalentTag == TalentTag;
		});
}
