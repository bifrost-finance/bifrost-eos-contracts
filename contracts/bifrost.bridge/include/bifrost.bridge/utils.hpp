namespace bifrost {

   void trim(string &s) {
      if (!s.empty()) {
         s.erase(0, s.find_first_not_of(" "));
         s.erase(s.find_last_not_of(" ") + 1);
      }
   }

   struct memo_info_type {
      string receiver;
      name peerchain;
      string notes;
   };

   memo_info_type get_memo_info(const string &memo_str) {
      static const string format = "{receiver}@{chain}:{memo}";
      memo_info_type info;

      string memo = memo_str;
      trim(memo);

      // --- get receiver ---
      auto pos = memo.find("@");
      check(pos != std::string::npos,
                   (string("memo format error, didn't find charactor \'@\' in memo, correct format: ") +
                    format).c_str());
      string receiver_str = memo.substr(0, pos);
      trim(receiver_str);
      info.receiver = receiver_str;

      // --- trim ---
      memo = memo.substr(pos + 1);
      trim(memo);

      // --- get chain name and notes ---
      pos = memo.find(":");
      if (pos == std::string::npos) {
         info.peerchain = name(memo);
         info.notes = "";
      } else {
         info.peerchain = name(memo.substr(0, pos));
         info.notes = memo.substr(pos + 1);
         trim(info.notes);
      }

      check(info.receiver.size() != 48, "invalid receiver length");
      check(info.peerchain != name(),(string("memo format error, chain not provided, correct format: ") + format).c_str());

      return info;
   }

}
